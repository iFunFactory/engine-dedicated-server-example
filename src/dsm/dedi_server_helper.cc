// Copyright (C) 2018 iFunFactory Inc. All Rights Reserved.
//
// This work is confidential and proprietary to iFunFactory Inc. and
// must not be used, disclosed, copied, or distributed without the prior
// consent of iFunFactory Inc.

#include <funapi/common/json.h>
#include "dedi_server_helper.h"


// 데디케이티드 서버 스폰 요청 타임아웃 시간(기본 값: 30초)을 지정합니다.
// 데디케이티드 서버 매니저는 스폰 요청을 받은 후 이 시간 동안 사용할 수 있는 서버를
// 찾지 못한 경우 실패를 반환합니다.
DECLARE_int32(dedicated_server_spawn_timeout);


namespace dsm {

namespace {

// 서버 스폰 관련 JSON 키 이름 정의
const char *kAccountId = "account_id";

const char *kUserData = "user_data";

const char *kMatchData = "match_data";

void OnDedicatedServerSpawned(const Uuid &match_id,
                              const std::vector<string> &account_ids,
                              bool success,
                              const SessionResponseHandler &handler) {
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

  LOG_ASSERT(account_ids.size() == 1);
  const string &account_id = *account_ids.begin();

  LOG(INFO) << "OnDedicatedServerSpawned"
            << ": match_id=" << match_id
            << ", account_ids=" << account_id
            << ", success=" << (success ? "succeed" : "failed");

  //
  // 클라이언트가 데디케이티드 서버로 접속하기 전에 처리해야 할 것이
  // 있을 경우, 이 곳에서 처리해야 합니다.
  const Ptr<Session> session = AccountManager::FindLocalSession(account_id);
  if (not session) {  // session = Session::kNullPtr
    // 다른 곳 또는 사용자가 세션을 닫아 로그아웃 한 상태입니다.
    handler(ResponseResult::FAILED,
            SessionResponse(Session::kNullPtr, 500,
                            "Internal server error.", Json()));
    return;
  }

  if (not success) {
    handler(ResponseResult::FAILED,
            SessionResponse(session, 500, "Internal server error.", Json()));
    return;
  }

  // 클라이언트에게 보낼 응답은 이 곳에 설정합니다.
  Json response_data;
  response_data["dedi_key1"] = "value1";
  response_data["dedi_key2"] = "value2";
  response_data["dedi_key3"] = "value3";

  handler(ResponseResult::OK,
          SessionResponse(session, 200, "OK", response_data));
}


void OnJoinedCallbackPosted(const Uuid &match_id,
                            const string &account_id) {
  LOG(INFO) << "OnJoinedCallbackPosted"
            << ": match_id=" << to_string(match_id)
            << ", account_id=" << account_id;
}


void OnLeftCallbackPosted(const Uuid &match_id,
                          const string &account_id) {
  LOG(INFO) << "OnLeftCallbackPosted"
            << ": match_id=" << to_string(match_id)
            << ", account_id=" << account_id;
}


void OnCustomCallbackPosted(const Uuid &match_id,
                            const Json &data) {
  LOG(INFO) << "OnCustomCallbackPosted"
            << ": match_id=" << to_string(match_id)
            << ", data=" << data.ToString(false);
}


void OnMatchResultPosted(const Uuid &match_id,
                         const Json &match_data,
                         bool success) {
  LOG(INFO) << "OnMatchResultPosted"
            << ": match_id=" << to_string(match_id)
            << ", success=" << (success ? "true" : "false")
            << ", match_data=" << match_data.ToString(false);
}

}  // unnamed namespace


void DediServerHelper::ProcessDediServerSpawn1(
    const Ptr<Session> &session,
    const Json &message,
    const SessionResponseHandler &handler) {
  LOG_ASSERT(session);
  LOG_ASSERT(handler);

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
  //   "match_data": {
  //      ...
  //   }
  // }

  // 메시지 안에 필수 파라메터가 있는지 확인합니다.
  if (not message.HasAttribute(kAccountId, Json::kString) ||
      not message.HasAttribute(kUserData, Json::kObject) ||
      not message.HasAttribute(kMatchData, Json::kObject)) {
    LOG(ERROR) << "The message does not have '" << kAccountId << "' or '"
               << kUserData << "'"
               << ": session_id=" << session->id()
               << ", message=" << message.ToString(false);
    handler(ResponseResult::FAILED,
            SessionResponse(session, 400, "Missing required fields.", Json()));
    return;
  }

  const string &account_id = message[kAccountId].GetString();
  const Json &data = message[kUserData];

  // 전달한 ID로 로그인한 세션이 있는지 확인합니다.
  const Ptr<Session> related_session =
      AccountManager::FindLocalSession(account_id);
  // 로그인을 하지 않은 경우 진행할 수 없습니다.
  if (not related_session) {
    LOG(ERROR) << "Could not find the session that is related to the account ID"
               << ": session_id=" << session->id()
               << ", message=" << message.ToString(false);
    handler(ResponseResult::FAILED,
            SessionResponse(session, 400, "Invalid arguments", Json()));
    return;
  }

  //
  // 데디케이티드 서버 인자 설정 가이드
  //

  // 1. 매치 ID
  // 방을 식별하는 용도로 사용합니다.
  // 여기서는 매치메이킹 기능을 사용하지 않으므로 직접 매치 ID 를 만들고 관리합니다.
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
  Json match_data = message[kMatchData];

  // TODO: 서버에서 'match_data' 오브젝트와 'data' 오브젝트가
  // 예측하는 범위 안의 값이나 설정 등을 사용하는지 반드시 확인해야 합니다.

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
  // response_handler 는 스폰 요청에 대한 응답만 하기 때문에
  // 데디케이티드 서버
  DedicatedServerManager::Spawn(
      match_id, match_data, dedicated_server_args, account_ids, user_data,
      bind(&OnDedicatedServerSpawned, _1, _2, _3, handler));
}


void DediServerHelper::RegisterHandler() {
  DedicatedServerManager::RegisterMatchResultCallback(OnMatchResultPosted);
  DedicatedServerManager::RegisterUserEnteredCallback(OnJoinedCallbackPosted);
  DedicatedServerManager::RegisterUserLeftCallback(OnLeftCallbackPosted);
  DedicatedServerManager::RegisterCustomCallback(OnCustomCallbackPosted);
}

}  // namespace dsm