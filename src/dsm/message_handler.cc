// Copyright (C) 2018 iFunFactory Inc. All Rights Reserved.
//
// This work is confidential and proprietary to iFunFactory Inc. and
// must not be used, disclosed, copied, or distributed without the prior
// consent of iFunFactory Inc.

#include "message_handler.h"

#include <funapi.h>
#include <functional>

//
// message_handler.cc 코드
//
// 이 파일은 클라이언트로부터 오는 모든 요청을 처리하는 곳입니다.
//
// RegisterMessageHandler() 안에서 등록한 메시지 타입(kLoginRequest 등)과
// 일치하는 요청이 왔을 때 이 곳에서 처리할 수 있습니다.
// 만약 등록하지 않은 메시지 타입으로 요청이 올 경우 엔진 내부적으로 에러 메시지를
// 출력합니다.
//
// 참고사항
//
// 1. 이 예제에서 세션과 관련있는 로그를 기록할 떄는 사용자를 추적할 수 있도록
// 항상 session ID 를 함께 출력합니다..
//
// 2. 클라이언트에 보낼 모든 메시지를 SendMyMessage() 안에서 처리하고 있습니다.
// 한 함수 안에서 응답을 처리하면 메시지를 일관성 있게 정의할 수 있습니다.
//

namespace dsm {

namespace {

// 로그인 관련 JSON 키 이름 정의
const char *kLoginRequest = "login";

const char *kAccountId = "player_id";

const char *kPlatformName = "platform";

const char *kFacebookAccessToken = "access_token";

// Spawn 관련 키 이름 정의
const char *kSpawnRequest = "spawn";

const char *kUserData = "user_data";


void SendMyMessage(const Ptr<Session> &session,
                   const int64_t code,
                   const string &message,
                   const Json &data) {
  //
  // 클라이언트에게 보낼 실패 응답을 이 곳에 설정합니다.
  //

  // 로직 상 NULL 세션 값이 오지 않도록 assert 를 추가했습니다.
  LOG_ASSERT(session);
  // data 는 반드시 object 형태만 포함합니다. value (문자열, 정수 등) 또는
  // 배열 형태의 JSON 데이터를 허용하지 않게 합니다.
  LOG_ASSERT(data.IsObject());
  // message 변수를 검사하지 않기 때문에 클라이언트는 빈 message 문자열을 받을 수
  // 있습니다. 만약 빈 문자열을 허용하고 싶지 않다면 아래 assert 를 추가해주세요.
  // LOG_ASSERT(not message.empty());

  Json response;
  response["error"]["code"] = code;
  response["error"]["message"] = message;
  response["data"] = data;

  session->SendMessage(kLoginRequest, response);
}


void OnLoggedIn(const string &account_id,
                const Ptr<Session> &session,
                bool logged_in,
                const string &platform,
                size_t try_count) {
  if (not logged_in) {
    if (try_count == 0) {
      // 로그인에 실패했습니다. 누군가 먼저 로그인 하거나, 시스템 장애일 수 있습니다.
      // 처음에는 강제로 로그아웃 후 재시도합니다.
      LOG(INFO) << "Already logged in"
                << ": session_id=" << session->id()
                << ", account_id=" << account_id
                << ", platform=" << platform
                << ", try_count=" << try_count;

      AccountManager::LogoutCallback on_logged_out =
          [platform, try_count](const string &account_id,
                                const Ptr<Session> &session,
                                bool logged_out) {
            // logged_out=false 라면 다른 곳에서 로그아웃을 한 경우입니다.

            // 로그아웃 후 어떻게 처리할지 결정합니다.
            // 이 예제에서는 별다른 처리 없이 다시 로그인을 시도합니다.
            AccountManager::CheckAndSetLoggedInAsync(
                account_id, session,
                bind(&OnLoggedIn, _1, _2, _3, platform, try_count));
          };

      // 분산 환경이라면 SetLoggedOutGlobalAsync() 함수를 사용해주세요.
      AccountManager::SetLoggedOutAsync(account_id, on_logged_out);
      return;
    } else {
      // 로그인을 두 번 이상 실패했습니다.
      // 만약 SetLoggedOutGlobalAsync() 함수를 사용 중이라면 분산 환경을
      // 구성하는 Zookeeper / Redis 서비스가 제대로 동작하지 않을 가능성도
      // 있습니다. 그러나 이 경우 엔진 내부적으로 조치한 후 에러를 출력하기
      // 때문에 여기서는 클라이언트 처리만 하는 게 좋습니다.
      LOG(ERROR) << "Login failed"
                 << ": session_id=" << session->id()
                 << ", account_id=" << account_id
                 << ", platform=" << platform
                 << ", try_count=" << try_count;

      SendMyMessage(session, 500, "internal server error.", Json());
      return;
    }
  }  // if (not logged_in)

  // 로그인 성공
  LOG(INFO) << "Login succeed"
            << ": session_id=" << session->id()
            << ", account_id=" << account_id
            << ", platform=" << platform
            << ", try_count=" << try_count;

  // 클라이언트에게 보낼 응답을 이 곳에 설정합니다.

  Json response_data;
  response_data["key1"] = "value1";
  response_data["key2"] = "value2";
  response_data["key3"] = "value3";

  SendMyMessage(session, 200, "OK", response_data);
}


void OnLoginRequest(const Ptr<Session> &session, const Json &message) {
  const SessionId &session_id = session->id();
  const Json &session_context = session->GetContext();

  LOG(INFO) << " OnLoginRequest"
            << ": session=" << session_id
            << ", context=" << session_context.ToString(false)
            << ", message=" << message.ToString(false);

  //
  // 로그인 요청 예제
  //
  // 클라이언트는 다음 메시지 형태로 로그인을 요청해야 합니다.
  // {
  //   // Facebook ID 또는 구글+ ID 등 고유한 ID 를 사용해야 합니다.
  //   "account_id": "id",
  //   "platform": "facebook"
  //   "access_token": "account's access token"
  // }

  // 메시지 안에 필수 파라메터가 있는지 확인합니다.
  if (not message.HasAttribute(kAccountId, Json::kString) ||
      not message.HasAttribute(kPlatformName, Json::kString) ) {
    LOG(ERROR) << "The message does not have '" << kAccountId << "' or '"
               << kPlatformName << "'"
               << ": session=" << session_id
               << ", message=" << message.ToString(false);
    SendMyMessage(session, 400, "Missing required fields.", Json());
    return;
  }

  const string &account_id = message[kAccountId].GetString();
  const string &platform = message[kPlatformName].GetString();

  if (platform == "facebook") {
    // Facebook 플랫폼 사용자의 경우, 올바른 사용자인지 검증합니다.
    if (not message.HasAttribute(kFacebookAccessToken, Json::kString)) {
      LOG(ERROR) << "The message does not have '" << kFacebookAccessToken
                 << ": session=" << session_id
                 << ", message=" << message.ToString(false);
      SendMyMessage(session, 400, "Missing required fields.", Json());
      return;
    }

    const string &access_token = message[kFacebookAccessToken].GetString();
    FacebookAuthenticationRequest request(access_token);

    FacebookAuthenticationResponseHandler on_authenticated =
        [session, account_id, platform](
            const FacebookAuthenticationRequest &request,
            const FacebookAuthenticationResponse &response,
            bool error) {
          // 인증 완료 후 어떻게 처리할 것인지 결정합니다.
          LOG_ASSERT(platform == "facebook");

          if (error) {
            // Facebook 서버 오류 또는 올바르지 않은 사용자인 경우
            LOG(WARNING) << "Failed to authenticate Facebook account"
                         << ": session=" << session->id()
                         << ", code=" << response.error.code
                         << ", message=" << response.error.message;
            SendMyMessage(session, 400, "Missing required fields.", Json());
            return;
          }

          LOG(INFO) << "Facebook authentication succeed"
                    << ": session=" << session->id()
                    << ", account_id=" << account_id;

          // 이 예제에서는 로그인 시도를 기록합니다.
          size_t try_count = 0;

          // 분산 환경이라면 CheckAndSetLoggedInGlobalAsync() 함수를 사용해주세요.
          AccountManager::CheckAndSetLoggedInAsync(
              account_id, session,
              bind(&OnLoggedIn, _1, _2, _3, platform, try_count));
        };

    // Facebook 인증을 요청합니다.
    Authenticate(request, on_authenticated);
  } else {
    //
    // 로그인 시도
    //
    // 요청한 세션으로 로그인을 시도합니다. 이 서버의 로그인 정책은 로그인을 시도하되,
    // 이미 다른 곳에서 로그인한 경우, 로그아웃 후 재시도합니다.
    size_t try_count = 0;

    // 분산 환경이라면 CheckAndSetLoggedInGlobalAsync() 함수를 사용해주세요.
    AccountManager::CheckAndSetLoggedInAsync(
        account_id, session,
        bind(&OnLoggedIn, _1, _2, _3, platform, try_count));
  }
}


void OnDedicatedServerSpawned(const Uuid &match_id,
                              const std::vector<string> &account_ids,
                              bool success) {
  //
  // 데디케이티드 서버 스폰 결과를 받는 콜백입니다.
  //
  // 데디케이티드 서버 매니저는 서버 프로세스 생성을 확인한 후, 매치 데이터를 전송합니다.
  // 데디케이티드 서버 프로세스가 매치 데이터를 정상적으로 받거나(success)/
  // 실패하면(not success) 데디케이티드 서버 매니저 가 이 콜백을 호출합니다.
  //
  // 데디케이티드 서버 매니저는 이 콜백 함수가 끝나는 즉시 클라이언트에게 데디케이티드
  // 서버 접속 메시지를 보냅니다. 즉 이 콜백 함수를 호출하는 시점에서 클라이언트는
  // 데디케이티드 서버와 연결하기 전입니다.
  //

  std::stringstream account_ids_ss;
  auto itr = account_ids.begin();
  auto itr_end = account_ids.end();
  account_ids_ss << "[";
  for (; itr != itr_end; ++itr) {
    if (itr != account_ids.begin()) {
      account_ids_ss << ", ";
    }
    account_ids_ss << "" << *itr;
  }
  account_ids_ss << "]";

  LOG(INFO) << "OnDedicatedServerSpawned"
            << ": match_id=" << match_id
            << ", account_ids=" << account_ids_ss.str()
            << ", success=" << (success ? "succeed" : "failed");

  if (not success) {
    return;
  }

  //
  // 매치 ID 를 별도로 저장하거나 관리하려면 이 곳에 코드를 추가해야 합니다.
  // 또는 클라이언트가 생성한 데디케이티드 서버로 접속하기 전에 처리해야 할 것이
  // 있을 경우, 이 곳에서 처리해야 합니다.
}


void OnSpawnRequest(const Ptr<Session> &session, const Json &message) {
  const SessionId &session_id = session->id();
  const Json &session_context = session->GetContext();

  LOG(INFO) << " OnSpawnRequest"
            << ": session=" << session_id
            << ", context=" << session_context.ToString(false)
            << ", message=" << message.ToString(false);

  //
  // 데디케이티드 서버 스폰 요청 예제 1 (방 생성)
  //
  // 유저 1명을 대상으로 새로운 데디케이티드 서버 프로세스를 생성합니다.
  // 프로세스 생성 후 다른 유저들은 데디케이티드 서버의 난입 기능을 통해 서버로
  // 접속해야 합니다.
  //
  // 클라이언트는 다음 메시지 형태로 데디케이티드 서버 스폰을 요청해야 합니다.
  // {
  //   // Facebook ID 또는 구글+ ID 등 고유한 ID 를 사용해야 합니다.
  //   "account_id": "id"
  //   "user_data": {
  //      ...
  //   },
  // }

  // 메시지 안에 필수 파라메터가 있는지 확인합니다.
  if (not message.HasAttribute(kAccountId, Json::kString) ||
      not message.HasAttribute(kUserData, Json::kObject) ) {
    LOG(ERROR) << "The message does not have '" << kAccountId << "' or '"
               << kUserData << "'"
               << ": session=" << session_id
               << ", message=" << message.ToString(false);
    SendMyMessage(session, 400, "Missing required fields.", Json());
    return;
  }

  const string &account_id = message[kAccountId].GetString();
  const Json &data = message[kUserData];

  //
  // 데디케이티드 서버 인자 설정 가이드
  //

  // 1. 매치 ID
  // 방을 식별하는 용도로 사용합니다.
  // 아래 함수를 사용하면 스폰 이후 방 정보를 가져올 수 있습니다.
  // DedicatedServerManager::GetGameState(
  //     const Uuid &match_id, const GetGameStateCallback &callback);
  //
  Uuid match_id = RandomGenerator::GenerateUuid();

  // 2. 매치 데이터
  // 이 매치에서 사용할 데이터를 넣습니다.
  // 이 데이터는 각 데디케이티드 서버에서 다음과 같이 받을 수 있습니다.
  // - 유니티 플러그인:
  //     - FunapiDedicatedServer.MatchDataCallback 에 등록한 콜백 함수
  // - 언리얼 엔진 플러그인:
  //     - SetMatchDataCallback 으로 등록한 콜백 함수
  //     - GetMatchDataJsonString() 함수
  Json match_data;
  match_data["match_type"] = "data 1";
  match_data["match_bonus_rate"] = 10;

  // 3. 데디케이티드 서버 인자
  // 데디케이티드 서버 프로세스 실행 시 함께 넘겨 줄 인자를 설정합니다.
  // 유니티, 언리얼 데디케이티드 서버에서 지원하는 인자가 있거나, 별도로
  // 인자를 지정해야 할 경우 이 곳에 입력하면 됩니다.
  const std::vector<string> dedicated_server_args {
      "-ExtraArg1=value1", "-ExtraArg2=value2", "-ExtraArg3=value3" };

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
  std::vector<string> account_ids { account_id };

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
  // 각 데디케이티드 서버는 클라이언트가
  std::vector<Json> user_data { data };

  // 두 컨테이너의 길이가 같아야 합니다. 0번 인덱스의 account_id 는 0번의 user_data
  // 를 사용하게 됩니다.
  LOG_ASSERT(account_ids.size() == user_data.size());

  // 준비한 인자들을 넣고 데디케이티드 서버 생성 요청을 합니다.
  DedicatedServerManager::Spawn(
      match_id, match_data, dedicated_server_args, account_ids, user_data,
      bind(&OnDedicatedServerSpawned, _1, _2, _3));
}

}  // unnamed namespace

void RegisterMessageHandler() {
  // 로그인 요청 핸들러를 등록합니다.
  HandlerRegistry::Register(kLoginRequest, OnLoginRequest);
  // 데디케이티드 서버 스폰(Spawn) 요청 핸들러를 등록합니다.
  HandlerRegistry::Register(kSpawnRequest, OnSpawnRequest);

}

}  // namespace dsm