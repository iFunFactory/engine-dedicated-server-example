// Copyright (C) 2018-2019 iFunFactory Inc. All Rights Reserved.
//
// This work is confidential and proprietary to iFunFactory Inc. and
// must not be used, disclosed, copied, or distributed without the prior
// consent of iFunFactory Inc.

#include "matchmaking_helper.h"

#include <funapi.h>

#include <src/dsm/matchmaking_type.h>


namespace dsm {

namespace {

// 매치메이킹 + 스폰 관련 JSON 키 이름 정의
const char *kAccountId = "account_id";

const char *kUserData = "user_data";

const char *kMatchType = "match_type";

const char *kMatchHistory = "match_history";


void LogMatchHistory(const Ptr<Session> &session,
                     const string &account_id,
                     const int64_t match_type) {
  // 추후 이 세션을 사용하는 다른 곳에서 세션 컨텍스트를 수정할 수 있습니다.
  // 따라서 세션 컨텍스트는 세션 ID 를 이벤트 태그로 하는 이벤트 위에서만
  // 수정해야 합니다(세션 열림, 닫힘, 메시지 핸들러는 기본적으로 ID를 태그로 사용합니다)
  LOG_ASSERT(GetCurrentEventTag() == session->id());
  Json &context = session->GetContext();
  if (not context.HasAttribute(kMatchHistory, Json::kObject)) {
    context[kMatchHistory].SetObject();
  }

  // 로그아웃 (AccountManager 를 쓸 수 없는) 상황에서도 접근할 수 있게
  // 계정 정보를 기록해둡니다.
  context[kMatchHistory][kAccountId] = account_id;
  context[kMatchHistory][kMatchType] = match_type;
}


void ClearMatchHistory(const Ptr<Session> &session) {
  // 추후 이 세션을 사용하는 다른 곳에서 세션 컨텍스트를 수정할 수 있습니다.
  // 따라서 세션 컨텍스트는 세션 ID 를 이벤트 태그로 하는 이벤트 위에서만
  // 수정해야 합니다(세션 열림, 닫힘, 메시지 핸들러는 기본적으로 ID를 태그로 사용합니다)
  LOG_ASSERT(GetCurrentEventTag() == session->id());
  Json &context = session->GetContext();

  if (context.HasAttribute(kMatchHistory, Json::kObject)) {
    context.RemoveAttribute(kMatchHistory);
  }
}


void OnMatchCompleted(const string &account_id,
                      const MatchmakingClient::Match &match,
                      MatchmakingClient::MatchResult result,
                      const Ptr<Session> &session,
                      const SessionResponseHandler &handler) {
  //
  // 매치 결과를 받는 콜백 핸들러입니다. 매치를 요청한 각 플레이어를 대상으로
  // 핸들러를 호출합니다.
  // 매치를 정상적으로 성사한 경우: match_id 로 직렬화 한 이벤트에서 실행합니다.
  // 매치 성사에 실패한 경우: 직렬화하지 않은 이벤트에서 실행합니다.
  //
  // 매치메이킹에 참여하는 사람이 많지 않거나 matchmaking_server_wrapper.cc 에서
  // 정의한 매치 조건이 까다로운 경우, 클라이언트가 매치메이킹을 요청하는 시점과
  // 매치가 성사되는 이 시점의 차가 커질 수 있습니다.
  //
  // 따라서 클라이언트는 매치메이킹 요청을 보낸 후 이 핸들러에서 메시지를 보내기
  // 전까지 다른 행동을 할 수 있도록 만들어야 합니다.
  //

  LOG(INFO) << "OnMatchCompleted"
            << ": account_id=" << account_id
            << ", result=" << result;

  if (result != MatchmakingClient::MatchResult::kMRSuccess) {
    // MatchmakingClient::MatchResult 결과에 따라 어떻게 처리할 지 결정합니다.
    if (result == MatchmakingClient::MatchResult::kMRError) {
      // 엔진 내부 에러입니다.
      // 일반적으로 RPC 서비스를 사용할 수 없을 경우 발생합니다.
      handler(ResponseResult::FAILED,
              SessionResponse(session, 500, "Internal server error.", Json()));
    } else if (result == MatchmakingClient::MatchResult::kMRAlreadyRequested) {
      // 이미 이 account_id 로 매치메이킹 요청을 했습니다. 매치 타입이 다르더라도
      // ID 가 같다면 실패합니다.
      handler(ResponseResult::FAILED,
              SessionResponse(session, 400, "Already requested.", Json()));
    } else if (result == MatchmakingClient::MatchResult::kMRTimeout) {
      // 매치메이킹 요청을 지정한 시간 안에 수행하지 못했습니다.
      // 더 넓은 범위의 매치 재시도 또는 클라이언트에게 단순 재시도를 요구할 수 있습니다.
      handler(ResponseResult::FAILED,
              SessionResponse(session, 400, "Timed out.", Json()));
    } else {
      // 아이펀 엔진에서 추후 MatchResult 를 더 추가할 경우를 대비해
      // 이곳에 로그를 기록합니다.
      LOG(WARNING) << "Not supported error: result=" << result;
      handler(ResponseResult::FAILED,
              SessionResponse(session, 500, "Unknown error.", Json()));
    }

    return;
  }

  // 매치 결과 ID 입니다.
  const MatchmakingClient::MatchId match_id = match.match_id;
  // 매치메이킹 서버에서 정의한 JSON 컨텍스트 입니다.
  // 매치메이킹 요청(StartMatchmaking2) 시 인자로 넣는 context 와는 다른
  // context 라는 점에 주의하세요.
  const Json &match_context = match.context;
  // 매치메이킹 요청 시 지정한 매치 타입입니다.
  const int64_t match_type = match.type;
  // 이 매치
  const std::vector<MatchmakingClient::Player> &players = match.players;

  std::stringstream players_ss;
  players_ss << "[";
  auto itr = players.begin();
  auto itr_end = players.end();
  for (; itr != itr_end; ++itr) {
    if (itr != players.begin()) {
      players_ss << ", ";
    }
    players_ss << "account_id=" << itr->id
               << ", user_data=" << itr->context.ToString(false);
  }
  players_ss << "]";

  LOG(INFO) << "Matchmaking succeed: match_id=" << match_id
            << ", match_context=" << match_context.ToString(false)
            << ", match_type=" << match_type
            << ", players_ss=" << players_ss.str();

  // 매치메이킹에 성공했습니다. 매치메이킹 서버(matchmaking_server_wrapper.cc)에서
  // 매치에 성공한 사람들을 모아 데디케이티드 서버 생성을 시작합니다.
  // (CheckMatchRequirements 함수에서 kMatchComplete 를 반환한 후입니다)
  // 이 시점에서는 단순히 클라이언트에게 매치메이킹 완료 메시지만 전달합니다.
  // (게임 로직에 따라 이 메시지 전송이 불필요할 수 있습니다)

  Event::Invoke([session, handler](){
    ClearMatchHistory(session);
    handler(ResponseResult::OK,
            SessionResponse(session, 200, "OK", Json()));
  }, session->id());
}


void OnMatchProgressUpdated(const string &account_id,
                            const MatchmakingClient::Match &match,
                            const string &player_id_joined,
                            const string &player_id_left) {
  LOG(INFO) << "OnMatchProgressUpdated: account_id=" << account_id
            << ", match_context=" << match.context.ToString(false)
            << ", player_id_joined=" << player_id_joined
            << ", player_id_left=" << player_id_left;

  // 누군가 매치에 참여하거나 나갔을 때 이 핸들러를 호출합니다.
  // 또는 다른 곳에서 MatchmakingClient::UpdateMatchPlayerContext() 를
  // 호출할 때도 이 핸들러를 호출합니다.
  //
  // 참고: UpdateMatchPlayerContext() 함수는 매치를 요청했던 사용자가
  // 자신이 보낸 user_data 내용을 변경할 때 사용할 수 있습니다.

  std::stringstream ss;
  const std::vector<MatchmakingClient::Player> &players = match.players;

  ss << "---------------------------------\n";
  auto itr = players.begin();
  auto itr_end = players.end();

  for (; itr != itr_end; ++itr) {
    if (itr != match.players.begin()) {
      ss << ", ";
    }
    ss << "account_id=" << itr->id
       << ", user_data=: " << itr->context.ToString(false);
  }
  ss << "\n---------------------------------";
  LOG(INFO) << ss.str();
}

}  // unnamed namespace


void MatchmakingHelper::ProcessMatchmaking(
    const Ptr<Session> &session,
    const Json &message,
    const SessionResponseHandler &handler) {
  LOG_ASSERT(session);
  // 메시지 핸들러 함수는 세션 ID 를 이벤트 태그로 하는 이벤트 위에서 실행합니다.
  // 아래는 이 함수를 메시지 핸들러 위에서 실행하게 강제하게 검사하는 식입니다.
  // 세션 ID 별로 이 함수를 직렬화하지 않으면 동시에 서로 다른 곳에서
  // 이 세션에 접근할 수 있습니다.
  LOG_ASSERT(GetCurrentEventTag() == session->id());

  //
  // 매치메이킹 + 데디케이티드 서버 스폰 요청 예제
  //
  // 유저 2명, 4명, 6명을 대상으로 새로운 데디케이티드 서버 프로세스를 생성합니다.
  //
  // 클라이언트는 다음 메시지 형태로 매치메이킹을 요청합니다.
  // {
  //   // Facebook ID 또는 구글+ ID 등 고유한 ID 를 사용해야 합니다.
  //   "account_id": "id",
  //   // 매치 타입, 이 파일에 있는 MatchType 정의 참조
  //   "match_type": 1
  //   "user_data": {
  //      "level": 70,
  //      "ranking_score": 1500,
  //      ...
  //   },
  // }

  if (not message.HasAttribute(kAccountId, Json::kString) ||
      not message.HasAttribute(kMatchType, Json::kInteger) ||
      not message.HasAttribute(kUserData, Json::kObject)) {
    LOG(ERROR) << "Missing required fields: '" << kAccountId << "' / '"
               << kMatchType << " / '" << kUserData << "'"
               << ": session_id=" << session->id()
               << ", message=" << message.ToString(false);
    handler(ResponseResult::FAILED,
            SessionResponse(session, 400, "Missing required fields.", Json()));
    return;
  }

  //
  // 매치메이킹 요청 인자 설정 가이드
  //

  // 1. 매치 타입
  // 매치 타입을 식별하는 용도로 사용합니다. 정해진 값이 없으므로
  // 서버에서 정의한 매치 타입과 일치하는지 확인할 필요가 있습니다.
  const int64_t match_type = message[kMatchType].GetInteger();
  if (not IsValidMatchType(match_type)) {
    LOG(ERROR) << "Invalid match_type"
               << ": session_id=" << session->id()
               << ", message=" << message.ToString(false);
    handler(ResponseResult::FAILED,
            SessionResponse(session, 400, "Invalid arguments.", Json()));
    return;
  }

  // 2. 계정(account) ID
  // AccountManager 로 로그인한 계정 ID 를 입력합니다.
  const string &account_id = message[kAccountId].GetString();

  // 요청한 계정과 로그인 중인 세션이 일치하는 지 검사합니다.
  // 이 검사를 통해 다른 유저 ID 로 매칭 요청하는 것을 방지할 수 있습니다.
  if (AccountManager::FindLocalSession(account_id)->id() != session->id()) {
    LOG(INFO) << "ProcessMatchmaking denied: bad request"
              << ": session_id=" << session->id()
              << ", message=" << message.ToString(false);
    handler(ResponseResult::FAILED,
            SessionResponse(session, 400,
                            "Access denied for this account.", Json()));
    return;
  }

  // 3. user_data, 매치메이킹 및 생성한 데디케이티드 서버 안에서 사용할
  // 플레이어의 데이터 입니다. 서버는 클라이언트에서 보낸 user_data 를 복사한 후
  // 요청 시간을 추가합니다. 따라서 매치메이킹 시 사용할 데이터는 최종적으로
  // 다음과 같습니다.
  //   "user_data": {
  //      "level": 70,
  //      "ranking_score": 1500,
  //      "req_time": 1544167339,
  //      ...
  //   },
  Json user_data = message[kUserData];
  if (not user_data.HasAttribute(kMatchLevel, Json::kInteger) ||
      not user_data.HasAttribute(kRankingScore, Json::kInteger)) {
    // 매치메이킹 요청에 필요한 인자가 부족합니다.
    LOG(ERROR) << "Missing required fields"
               << ": session_id=" << session->id()
               << ", message=" << message.ToString(false);
    handler(ResponseResult::FAILED,
            SessionResponse(session, 400, "Missing required fields.", Json()));
    return;
  }

  // 4. OnMatchCompleted 콜백
  // 매칭을 종료했을 때 결과를 받을 콜백 함수를 지정합니다.
  // 매치 결과는 콜백 함수의 MatchmakingClient::MatchResult 타입으로
  // 확인할 수 있습니다.

  // 5. 서버 선택 방법
  //    - kRandom: 서버를 랜덤하게 선택합니다.
  //    - kMostNumberOfPlayers; 사람이 가장 많은 서버에서 매치를 수행합니다.
  //    - kLeastNumberOfPlayers; 사람이 가장 적은 서버에서 매치를 수행합니다.
  const MatchmakingClient::TargetServerSelection target_selection =
      MatchmakingClient::kRandom;

  // 6. OnMatchProgressUpdated 콜백
  // 매치 상태를 업데이트 할 때마다 결과를 받을 콜백 함수를 지정합니다.
  // 타임 아웃 (기본 값: kNullTimeout)
  const WallClock::Duration timeout = MatchmakingClient::kNullTimeout;

  // 이 세션에서 요청한 매치 타입을 기록해둡니다.
  LogMatchHistory(session, account_id, match_type);

  LOG(INFO) << "Requesting a matchmaking"
            << ": session_id=" << session->id()
            << ", account_id=" << account_id
            << ", match_type=" << match_type
            << ", user_data=" << user_data.ToString(false);

  MatchmakingClient::StartMatchmaking2(
      match_type, account_id, user_data,
      bind(&OnMatchCompleted, _1, _2, _3, session, handler),
      target_selection,
      OnMatchProgressUpdated,
      timeout);
}


void MatchmakingHelper::CancelMatchmaking(
    const Ptr<Session> &session,
    const Json &message,
    const SessionResponseHandler &handler) {
  // 메시지 핸들러 함수는 세션 ID 를 이벤트 태그로 하는 이벤트 위에서 실행합니다.
  // 아래는 이 함수를 메시지 핸들러 위에서 실행하게 강제하게 검사하는 식입니다.
  // 세션 ID 별로 이 함수를 직렬화하지 않으면 동시에 서로 다른 곳에서
  // 이 세션에 접근할 수 있습니다.
  LOG_ASSERT(GetCurrentEventTag() == session->id());

  // 클라이언트는 다음 메시지 형태로 매치메이킹 취소를 요청합니다.
  // {
  //   "account_id": "id",
  //   "match_type": 1
  // }

  if (not message.HasAttribute(kAccountId, Json::kString) ||
      not message.HasAttribute(kMatchType, Json::kInteger)) {
    LOG(ERROR) << "Missing required fields: '" << kAccountId << "' / '"
               << kMatchType << "'" << ": session_id=" << session->id()
               << ", message=" << message.ToString(false);
    handler(ResponseResult::FAILED,
            SessionResponse(session, 400, "Missing required fields.", Json()));
    return;
  }

  // 매치 타입
  const int64_t match_type = message[kMatchType].GetInteger();
  if (not IsValidMatchType(match_type)) {
    LOG(ERROR) << "Invalid match_type"
               << ": session_id=" << session->id()
               << ", message=" << message.ToString(false);
    handler(ResponseResult::FAILED,
            SessionResponse(session, 400, "Invalid arguments.", Json()));
    return;
  }

  // 계정
  const string &account_id = message[kAccountId].GetString();

  // 요청한 계정과 로그인 중인 세션이 일치하는 지 검사합니다.
  // 이 검사를 통해 다른 유저 ID 로 매칭을 취소하는 행위를 방지할 수 있습니다.
  if (AccountManager::FindLocalSession(account_id)->id() != session->id()) {
    LOG(INFO) << "CancelMatchmaking denied: bad request"
              << ": session_id=" << session->id()
              << ", message=" << message.ToString(false);
    handler(ResponseResult::FAILED,
            SessionResponse(session, 400,
                            "Access denied for this account.", Json()));
    return;
  }

  MatchmakingClient::CancelCallback cancel_callback =
      [session, handler](const string &account_id,
                         MatchmakingClient::CancelResult result) {
        if (result != MatchmakingClient::kCRSuccess) {
          // kCRNoRequest (요청하지 않은 매치) 또는
          // kCRError (엔진 내부 에러)가 올 수 있습니다.
          handler(ResponseResult::FAILED,
                  SessionResponse(session, 500, "Internal server error.",
                                  Json()));
        } else {
          handler(ResponseResult::OK,
                  SessionResponse(session, 200, "OK.", Json()));
        }
      };

  LOG(INFO) << "Canceling matchmaking"
            << ": session_id=" << session->id()
            << ", account_id=" << account_id
            << ", match_type=" << match_type;

  MatchmakingClient::CancelMatchmaking(
      match_type, account_id, cancel_callback);
}


void MatchmakingHelper::CancelMatchmaking(const Ptr<Session> &session) {
  // 메시지 핸들러 함수는 세션 ID 를 이벤트 태그로 하는 이벤트 위에서 실행합니다.
  // 아래는 이 함수를 메시지 핸들러 위에서 실행하게 강제하게 검사하는 식입니다.
  // 세션 ID 별로 이 함수를 직렬화하지 않으면 동시에 서로 다른 곳에서
  // 이 세션에 접근할 수 있습니다.
  LOG_ASSERT(GetCurrentEventTag() == session->id());

  Json &context = session->GetContext();
  if (not context.HasAttribute(kMatchHistory, Json::kObject)) {
    // 매칭을 요청한 적이 없습니다. 종료합니다.
    return;
  }

  LOG_ASSERT(context[kMatchHistory].HasAttribute(kAccountId, Json::kObject));
  LOG_ASSERT(context[kMatchHistory].HasAttribute(kMatchType, Json::kObject));

  const string &account_id = context[kMatchHistory][kAccountId].GetString();
  const int64_t match_type = context[kMatchHistory][kMatchType].GetInteger();

  MatchmakingClient::CancelCallback cancel_callback =
      [](const string &account_id, MatchmakingClient::CancelResult result) {
    // 아무것도 하지 않습니다.
  };

  LOG(INFO) << "Canceling matchmaking(with session context)"
            << ": session_id=" << session->id()
            << ", account_id=" << account_id
            << ", match_type=" << match_type;

  MatchmakingClient::CancelMatchmaking(
      match_type, account_id, cancel_callback);
}

}  // namespace dsm
