// Copyright (C) 2018-2019 iFunFactory Inc. All Rights Reserved.
//
// This work is confidential and proprietary to iFunFactory Inc. and
// must not be used, disclosed, copied, or distributed without the prior
// consent of iFunFactory Inc.

#include "matchmaking_server_wrapper.h"

#include <funapi.h>
#include <glog/logging.h>

#include <src/dsm/matchmaking_type.h>


namespace dsm {

namespace {

const char *kBlueTeam = "blue_team";

const char *kRedTeam = "red_team";


SessionResponseHandler the_response_handler;


void OnDedicatedServerSpawned(const Uuid &match_id,
                              const std::vector<string> &account_ids,
                              bool success) {
  LOG_ASSERT(the_response_handler);
  //
  // 데디케이티드 서버 스폰 결과를 받는 콜백 핸들러입니다.
  //
  // 대기 중인 유후 서버가 있다면 데디케이티드 서버 매니저는 이 핸들러를 거의 즉시
  // 호출하지만, 대기 중인 서버가 없다면 최대 FLAGS_dedicated_server_spawn_timeout
  // 초 만큼 기다린 후 이 핸들러를 호출할 수 있습니다 (success=false).
  //
  // 데디케이티드 서버 매니저는 이 콜백 함수가 끝나는 즉시 클라이언트에게 데디케이티드
  // 서버 접속 메시지를 보냅니다. 즉, 데디케이티드 서버 매니저는 이 콜백 함수를 호출하는
  // 시점에서 클라이언트가 데디케이티드 서버가 연결하기 전이라는 것을 보장합니다.
  //
  // 다음은 데디케이티드 서버 스폰 실패 시 확인해야 할 내용들입니다.
  //
  // - 접속 가능하고 매치를 받을 수 있는 데디케이티드 서버 호스트가 있는지 확인해주세요.
  //   - Redis 의 ife-dedi-hosts 키에 등록된 호스트가 있는지 확인해주세요.
  //   - AWS 를 사용할 경우 FLAGS_dedicated_server_spawn_timeout 이 충분히 길어야
  //     합니다. 그렇지 않으면 새 서버를 추가하는 동안 타임 아웃이 날 수 있습니다)
  // - 데디케이티드 서버 실행 시 인지를 제대로 주었는지 확인해주세요.
  //   - 유니티/언리얼 데디케이티드 서버는 정상적으로 실행했으나, 초기화(Ready)를
  //     정상적으로 수행하지 못했거나, 프로세스가 크래시 했을 가능성도 있습니다.
  //

  LOG(INFO) << "OnDedicatedServerSpawned"
            << ": match_id=" << match_id
            << ", success=" << (success ? "succeed" : "failed");

  for (const auto &account_id : account_ids) {
    // 각 클라이언트가 데디케이티드 서버로 접속하기 전에 처리해야 할 것이
    // 있을 경우, 이 곳에서 처리해야 합니다.
    const Ptr<Session> &session = AccountManager::FindLocalSession(account_id);
    if (not session) {  // session = Session::kNullPtr
      // 다른 곳 또는 사용자가 세션을 닫거나 로그아웃 한 상태입니다.
      return;
    }

    if (not success) {
      the_response_handler(ResponseResult::FAILED,
          SessionResponse(session, 500, "Internal server error.", Json()));
      return;
    }

    // 클라이언트에게 보낼 응답은 이 곳에 설정합니다.
    Json response_data;
    response_data["match_then_dedi_key1"] = "match_then_dedi_value1";
    response_data["match_then_dedi_key2"] = "match_then_dedi_value2";
    response_data["match_then_dedi_key3"] = "match_then_dedi_value3";

    the_response_handler(ResponseResult::OK,
           SessionResponse(session, 200, "OK", response_data));
  }  // for (const auto &account_id : account_ids)
}


void SpawnDediServer(const MatchmakingServer::Match &match) {
  //
  // 데디케이티드 서버 인자 설정 가이드
  //

  // 1. 매치 ID
  // 방을 식별하는 용도로 사용합니다.
  // 여기서는 매치메이킹 기능을 사용하므로 매치 ID 를 그대로 사용합니다.
  // 아래 함수를 사용하면 스폰 이후 방 정보를 가져올 수 있습니다.
  // DedicatedServerManager::GetGameState(
  //     const Uuid &match_id, const GetGameStateCallback &callback);
  //
  const Uuid match_id = match.match_id;

  // 2. 매치 데이터
  // 이 매치에서 사용할 데이터를 넣습니다.
  // 이 데이터는 각 데디케이티드 서버에서 다음과 같이 받을 수 있습니다.
  // - 유니티 플러그인:
  //     - FunapiDedicatedServer.MatchDataCallback 에 등록한 콜백 함수
  // - 언리얼 엔진 플러그인:
  //     - SetMatchDataCallback 으로 등록한 콜백 함수
  //     - GetMatchDataJsonString() 함수
  //
  // 이 예제에서는 매치 타입에 따른 데이터를 넣습니다.
  Json match_data;
  if (match.type == kMatch1vs1) {
    match_data["type"] = kMatch1vs1;
  } else if (match.type == kMatch3v3) {
    match_data["type"] = kMatch3v3;
  } else if (match.type == kMatch6v6) {
    match_data["type"] = kMatch3v3;
  }

  // 3. 데디케이티드 서버 인자
  // 데디케이티드 서버 프로세스 실행 시 함께 넘겨 줄 인자를 설정합니다.
  // 유니티, 언리얼 데디케이티드 서버에서 지원하는 인자가 있거나, 별도로
  // 인자를 지정해야 할 경우 이 곳에 입력하면 됩니다.
  const std::vector<string> dedicated_server_args {"HighRise?game=FFA", "-log"};

  // 4. account_ids
  // 데디케이티드 서버 프로세스 생성 시 함께 전달할 계정 ID를 추가합니다.
  //
  // 게임 클라이언트는 데디케이티드 서버 프로세스 생성 후 리다이렉션 요청을 받으며
  // 이후 데디케이티드 서버로 진입하게 됩니다. 데디케이티드 서버는 이 때 클라이언트가
  // 올바른 account_id 와 token 을 가지고 있는지 다음과 같이 검증할 수 있습니다.
  // - 유니티 플러그인:
  //     - FunapiDedicatedServer.AuthUser(string uid, string token)
  // - 언리얼 플러그인:
  //     - bool FunapiDedicatedServer::AuthUser(
  //         const FString& options, const FString& uid_field,
  //         const FString& token_field, FString &error_message);
  //
  std::vector<string> account_ids;
  for (const MatchmakingClient::Player &player : match.players) {
    account_ids.push_back(player.id);
  }

  // 5. user_data
  // 데디케이티드 서버 프로세스 생성 시 함께 전달할 유저 데이터를 추가합니다.
  // 유저 데이터는 각 데디케이티드 서버에서 다음과 같이 받을 수 있습니다.
  // (uid == account_id 와 동일합니다)
  // - 유니티 플러그인:
  //     - FunapiDedicatedServer.UserDataCallback 에 등록한 콜백 함수
  //     - FunapiDedicatedServer.GetUserDataJsonString(string uid) 함수
  // 언리얼 플러그인:
  //     - FunapiDedicatedServer::SetUserDataCallback 으로 등록한 콜백 함수
  //     - FunapiDedicatedServer::GetUserDataJsonString(const FString &uid) 함수
  //
  // 이 예제에서는 matchmaking 에서 제공하는 user_context 의 user_data 영역
  // ( MatchmakingClient::StartMatchmaking2 에서 지정한 user_data )
  // 을 추가합니다.
  std::vector<Json> user_data;
  for (const MatchmakingClient::Player &player : match.players) {
    user_data.push_back(player.context["user_data"]);
  }

  // 두 컨테이너의 길이가 같아야 합니다. 0번 인덱스의 account_id 는 0번의 user_data
  // 를 사용하게 됩니다.
  LOG_ASSERT(account_ids.size() == user_data.size());

  // 준비한 인자들을 넣고 데디케이티드 서버 생성 요청을 합니다.
  // response_handler 는 스폰 요청에 대한 응답만 하기 때문에
  // 데디케이티드 서버
  DedicatedServerManager::Spawn(
      match_id, match_data, dedicated_server_args, account_ids, user_data,
      bind(&OnDedicatedServerSpawned, _1, _2, _3));
}


bool CheckPlayerRequirements(const MatchmakingServer::Player &player,
                             const MatchmakingServer::Match &match) {
  //
  // 매치 조건 검사 핸들러 함수
  //

  // 플레이어가 이 매치에 참여해도 괜찮은 지 검사합니다.
  // 다음 링크를 참고해 단계 별 매칭, 그룹 매칭 등 여러 조건에 대한 예제를
  // 확인할 수 있습니다.
  // https://ifunfactory.com/engine/documents/reference/ko/contents-support-matchmaking.html#id17

  const string &account_id = player.id;

  //
  // 메치메이킹 서버는 StartMatchmaking2 에서 넣은 데이터를 다음 형태로 가공합니다.
  // request_time, elapsed_time 값은 엔진에서 자동으로 추가하고 관리합니다.
  //
  //
  // {
  //   "request_time":6447934119, // 요청 시각
  //   "elapsed_time":0,          // 요청 이후 지난 시간(단위: 초)
  //   "user_data": {
  //     // StartMatchmaking2() 안에 넣은 user_data
  //   }
  //}
  //
  const Json &user_context = player.context;
  const Json &user_data = player.context["user_data"];

  LOG_ASSERT(user_context.HasAttribute("request_time", Json::kInteger));
  LOG_ASSERT(user_context.HasAttribute("elapsed_time", Json::kInteger));

  LOG_ASSERT(user_data.HasAttribute(kMatchLevel, Json::kInteger))
      << ": user_data=" << user_data.ToString(false);
  LOG_ASSERT(user_data.HasAttribute(kRankingScore, Json::kInteger))
      << ": user_data=" << user_data.ToString(false);

  const std::vector<MatchmakingClient::Player> &players = match.players;

  LOG(INFO) << "Checking the first triger...";

  // 조건 1. 매치 상대가 없으면 (자신만 있다면) 바로 매치에 참여합니다.
  if (players.size() == 1) {
    LOG(INFO) << "[Trigger 1] A new player is going to join the match"
              << ": match_id=" << match.match_id
              << ", account_id=" << player.id
              << ", user_context=" << player.context.ToString(false);
    return true;
  }

  LOG(INFO) << "Checking the second trigger...";

  // 조건 2. 매치메이킹에 참여중인 플레이어 중 1명이 30초 이상 기다린 경우 바로 넣습니다.
  auto itr = players.begin();
  const auto itr_end = players.end();
  for (; itr != itr_end; ++itr) {
    if (itr->id == account_id) {
      // 조건을 검사할 때 나 자신은 제외합니다.
      continue;
    }

    const Json &user_context2 = itr->context;
    const int64_t elapsed_sec = user_context2["elapsed_time"].GetInteger();
    if (elapsed_sec >= 30) {
      LOG(INFO) << "[Trigger 2] A new player is going to join the match"
                << ": match_id=" << match.match_id
                << ", account_id=" << player.id
                << ", user_context=" << player.context.ToString(false);
      return true;
    }
  }

  LOG(INFO) << "Checking the third trigger...";

  // 조건 3. 평균 레벨 차가 10 이상이거나, 랭킹 점수 차가 100점 이상인 경우
  // 매치에 참여시키지 않습니다.
  const int64_t my_match_level = user_data[kMatchLevel].GetInteger();
  const int64_t my_ranking_score = user_data[kRankingScore].GetInteger();

  int64_t avg_match_level = 0, avg_ranking_score = 0;

  itr = players.begin();
  for (; itr != itr_end; ++itr) {
    if (itr->id == account_id) {
      // 조건을 검사할 때 나 자신은 제외합니다.
      continue;
    }

    const Json &user_context2 = itr->context;
    const Json &user_data2 = user_context2["user_data"];
    avg_match_level += user_data2[kMatchLevel].GetInteger();
    avg_ranking_score += user_data2[kRankingScore].GetInteger();
  }

  avg_match_level /= (players.size() - 1);
  avg_ranking_score /= (players.size() - 1);

  LOG(INFO) << "avg_match_level=" << avg_match_level
            << ", avg_ranking_score=" << avg_ranking_score;

  if (labs(my_match_level - avg_match_level) >= 10 /* 10점 이상 */ ||
      labs(my_ranking_score - avg_ranking_score) >= 100 /* 100점 이상*/) {
    return false;
  }

  // 매치 조건에 만족하는 플레이어를 찾았습니다.
  LOG(INFO) << "[Trigger 3] A new player is going to join the match"
            << ": match_id=" << match.match_id
            << ", account_id=" << player.id
            << ", user_context=" << player.context.ToString(false);
  return true;
}


MatchmakingServer::MatchState CheckMatchRequirements(
    const MatchmakingServer::Match &match) {
  //
  // 매치 완료 조건 검사 핸들러 함수입니다.
  //

  size_t total_players_for_match = 0;
  if (match.type == kMatch1vs1) {
    total_players_for_match = 2;
  } else if (match.type == kMatch3v3) {
    total_players_for_match = 6;
  } else if (match.type == kMatch6v6) {
    total_players_for_match = 12;
  }

  // 위 조건과 다른 매치 타입이 들어올 수 없으므로 이를 검사합니다.
  LOG_ASSERT(total_players_for_match != 0);

  // 총 플레이어 수가 매치 완료 조건에 부합하는 지 검사합니다.
  if (match.players.size() != total_players_for_match) {
    // 아직 더 많은 플레이어가 필요합니다.
    LOG(INFO) << "Waiting for more players"
              << ": match_id=" << match.match_id
              << ", match_type=" << match.type
              << ", total_players_for_match=" << total_players_for_match
              << ", current players=" << match.players.size();

    return MatchmakingServer::kMatchNeedMorePlayer;
  }

  // 매치메이킹이 끝났으니 이 정보를 토대로 데디케이티드 서버 생성을 요청합니다.
  SpawnDediServer(match);

  return MatchmakingServer::kMatchComplete;
}


void OnPlayerJoined(const MatchmakingServer::Player &player,
                    MatchmakingServer::Match *match) {
  //
  // 플레이어를 매치에 포함한 후 호출하는 핸들러 함수 입니다.
  //
  // CheckPlayerRequirements() 함수 안에서 true 를 반환하면 이 함수를 호출합니다.
  //
  // 이 예제에서는 레드 / 블루로 나눠진 팀에 각각 플레이어를 넣습니다.
  // 블루 팀 인원수가 레드 팀 인원 수보다 많지 않는 한 레드 팀에 우선적으로
  // 플레이어를 넣습니다.

  // 매치를 처음 생성할 때는 매치 컨텍스트가 비어있는 상태이므로 초기화가 필요합니다.
  if (match->context.IsNull()) {
    match->context.SetObject();
    match->context[kBlueTeam].SetArray();
    match->context[kRedTeam].SetArray();
  }

  if (match->context[kBlueTeam].Size() > match->context[kRedTeam].Size()) {
    match->context[kRedTeam].PushBack(player.id);
  } else {
    match->context[kBlueTeam].PushBack(player.id);
  }
}


void OnPlayerLeft(const MatchmakingServer::Player &player,
                  MatchmakingServer::Match *match) {
  LOG_ASSERT(not match->context.IsNull());
  LOG_ASSERT(match->context[kBlueTeam].IsArray());
  LOG_ASSERT(match->context[kRedTeam].IsArray());

  {
    // 매치메이킹 도중 나간 플레이어가 블루 팀에 있는지 확인합니다.
    Json::ValueIterator itr = match->context[kBlueTeam].Begin();
    Json::ConstValueIterator itr_end = match->context[kBlueTeam].End();
    for (; itr != itr_end; ++itr) {
      if (itr->GetString() == player.id) {
        itr->RemoveElement(itr);
        return;
      }
    }
  }

  {
    // 매치메이킹 도중 나간 플레이어가 레드 팀에 있는지 확인합니다.
    Json::ValueIterator itr = match->context[kRedTeam].Begin();
    Json::ConstValueIterator itr_end = match->context[kRedTeam].End();
    for (; itr != itr_end; ++itr) {
      if (itr->GetString() == player.id) {
        itr->RemoveElement(itr);
        return;
      }
    }
  }
}

}  // unnamed namespace


void MatchmakingServerWrapper::Install(
    const SessionResponseHandler &response_handler) {
  the_response_handler = response_handler;
}


void MatchmakingServerWrapper::Start() {
  MatchmakingServer::Start(
      CheckPlayerRequirements, CheckMatchRequirements,
      OnPlayerJoined, OnPlayerLeft);
}

}  // namespace dsm
