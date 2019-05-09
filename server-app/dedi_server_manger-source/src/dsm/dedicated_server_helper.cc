// Copyright (C) 2018 iFunFactory Inc. All Rights Reserved.
//
// This work is confidential and proprietary to iFunFactory Inc. and
// must not be used, disclosed, copied, or distributed without the prior
// consent of iFunFactory Inc.

#include <funapi/common/json.h>
#include "dedicated_server_helper.h"
#include "matchmaking_type.h"

// 데디케이티드 서버 스폰 요청 타임아웃 시간(기본 값: 30초)을 지정합니다.
// 데디케이티드 서버 매니저는 스폰 요청을 받은 후 이 시간 동안 사용할 수 있는 서버를
// 찾지 못한 경우 실패를 반환합니다.
DECLARE_int32(dedicated_server_spawn_timeout);


namespace dsm {

namespace {

const char *kMatchType = "match_type";

// 현재 진행중인 매치를 관리합니다. 이 정보는 난입할 수 있는 서버를
// 조사할 때 사용합니다. 게임 서버가 한 대 이상인 경우 Redis 와 같은 곳에
// 키를 두고 매치 정보를 저장해야 합니다.

struct MyMatchInfo {
  Uuid match_id;
  int64_t match_type;
  Json match_data;
  std::set<string /*player_id*/> players;
};

boost::mutex the_match_mutex;
std::map<Uuid /*match_id*/, MyMatchInfo> match_map;
std::map<int64_t /*match_type*/, std::set<Uuid>> match_type_map;


SessionResponseHandler the_response_handler;


void OnDedicatedServerSpawned(const Uuid &match_id,
                              const std::vector<string> &account_ids,
                              bool success,
                              const Json &match_data,
                              int64_t match_type) {
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
  if (success) {
    boost::mutex::scoped_lock lock(the_match_mutex);
    MyMatchInfo info { match_id, match_type, match_data };
    LOG_ASSERT(match_map.emplace(match_id, info).second);
    LOG_ASSERT(match_type_map[match_type].emplace(match_id).second);
  }

  for (auto &account_id : account_ids) {
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


void OnUserSent(const Uuid &match_id,
                const std::vector<string> &account_ids,
                bool success,
                const Json &match_data,
                int64_t match_type,
                const DedicatedServerHelper::SendUserCallback &cb) {
  LOG_ASSERT(cb);

  // 난입 결과를 는 콜백 핸들러입니다.
  // 데디케이티드 서버 매니저는 이 콜백 함수가 끝나는 즉시 클라이언트에게 데디케이티드
  // 서버 접속 메시지를 보냅니다. 즉, 데디케이티드 서버 매니저는 이 콜백 함수를 호출하는
  // 시점에서 클라이언트가 데디케이티드 서버가 연결하기 전이라는 것을 보장합니다.

  LOG(INFO) << "OnUserSent"
            << ": match_id=" << match_id
            << ", success=" << (success ? "succeed" : "failed");
  cb(success);
  for (auto &account_id : account_ids) {
    // 각 클라이언트가 데디케이티드 서버로 접속하기 전에 처리해야 할 것이
    // 있을 경우, 이 곳에서 처리해야 합니다.
    const Ptr<Session> &session = AccountManager::FindLocalSession(account_id);
    if (not session) {  // session = Session::kNullPtr
      // 다른 곳 또는 사용자가 세션을 닫거나 로그아웃 한 상태입니다.
      return;
    }
  }
}


void OnJoinedCallbackPosted(const Uuid &match_id,
                            const string &account_id) {
  LOG(INFO) << "OnJoinedCallbackPosted"
            << ": match_id=" << to_string(match_id)
            << ", account_id=" << account_id;

  // 누군가 데디케이티드 서버로 접속했습니다(SendJoin 함수를 호출했습니다).
  // 유저를 추가합니다.
  {
    boost::mutex::scoped_lock lock(the_match_mutex);
    auto itr = match_map.find(match_id);
    if (itr == match_map.end()) {
      // 끝난 게임의 match_id로 호출했습니다.(OnMatchResultPosted가 호출된 후)
      LOG(WARNING) << "Match does not exist."
                   << ": match_id=" << to_string(match_id);
      return;
    }
    itr->second.players.emplace(account_id);
  }
}


void OnLeftCallbackPosted(const Uuid &match_id,
                          const string &account_id) {
  LOG(INFO) << "OnLeftCallbackPosted"
            << ": match_id=" << to_string(match_id)
            << ", account_id=" << account_id;

  // 누군가 데디케이티드 서버에서 나갔습니다(SendLeft 함수를 호출했습니다).
  // 유저를 제거합니다.
  {
    boost::mutex::scoped_lock lock(the_match_mutex);
    auto itr = match_map.find(match_id);
    if (itr == match_map.end()) {
      // 끝난 게임의 match_id로 호출했습니다.(OnMatchResultPosted가 호출된 후)
      LOG(WARNING) << "Match does not exist."
                   << ": match_id=" << to_string(match_id);
      return;
    }
    itr->second.players.erase(account_id);
  }
}


void OnCustomCallbackPosted(const Uuid &match_id,
                            const Json &data) {
  LOG(INFO) << "OnCustomCallbackPosted"
            << ": match_id=" << to_string(match_id)
            << ", data=" << data.ToString(false);
  // 데디케이티드 서버에서 CustomCallback 을 호출했습니다.
}


void OnMatchResultPosted(const Uuid &match_id,
                         const Json &match_data,
                         bool success) {
  LOG(INFO) << "OnMatchResultPosted"
            << ": match_id=" << to_string(match_id)
            << ", success=" << (success ? "true" : "false")
            << ", match_data=" << match_data.ToString(false);

  // 데디케이티드 서버에서 게임이 끝났고 결과를 받았습니다.
  // match_data 를 필요게 맞게 가공해 데이터베이스에 저장하거나 할 수 있습니다.
  {
    boost::mutex::scoped_lock lock(the_match_mutex);
    auto itr = match_map.find(match_id);
    LOG_ASSERT(itr != match_map.end());

    match_map.erase(match_id);
    match_type_map[itr->second.match_type].erase(match_id);
  }
}

}  // unnamed namespace


void DedicatedServerHelper::SpawnDedicatedServer(
    const string &account_id,
    const Json &user_data) {
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
  const Uuid match_id = RandomGenerator::GenerateUuid();

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
  match_data[kMatchType] = kNoMatching;

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
  //
  // 이 예제에서는 matchmaking 에서 제공하는 user_context 의 user_data 영역
  // ( MatchmakingClient::StartMatchmaking2 에서 지정한 user_data )
  // 을 추가합니다.
  std::vector<Json> user_data_list { user_data };

  // 두 컨테이너의 길이가 같아야 합니다. 0번 인덱스의 account_id 는 0번의 user_data
  // 를 사용하게 됩니다.
  LOG_ASSERT(account_ids.size() == user_data_list.size());

  // 준비한 인자들을 넣고 데디케이티드 서버 생성 요청을 합니다.
  // response_handler 는 스폰 요청에 대한 응답만 하기 때문에
  // 데디케이티드 서버
  DedicatedServerManager::Spawn(
      match_id, match_data, dedicated_server_args,
      account_ids, user_data_list,
      bind(&OnDedicatedServerSpawned, _1, _2, _3, match_data, kNoMatching));
}


void DedicatedServerHelper::SpawnDedicatedServer(
    const MatchmakingServer::Match &match) {
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
  match_data[kMatchType] = match.type;

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
  std::vector<Json> user_data_list;
  for (const MatchmakingClient::Player &player : match.players) {
    user_data_list.push_back(player.context["user_data"]);
  }

  // 두 컨테이너의 길이가 같아야 합니다. 0번 인덱스의 account_id 는 0번의 user_data
  // 를 사용하게 됩니다.
  LOG_ASSERT(account_ids.size() == user_data_list.size());

  // 준비한 인자들을 넣고 데디케이티드 서버 생성 요청을 합니다.
  // response_handler 는 스폰 요청에 대한 응답만 하기 때문에
  // 데디케이티드 서버
  DedicatedServerManager::Spawn(
      match_id, match_data, dedicated_server_args,
      account_ids, user_data_list,
      bind(&OnDedicatedServerSpawned, _1, _2, _3, match_data, match.type));
}


void DedicatedServerHelper::SendUser(
    int64_t match_type,
    const string &account_id,
    const Json &user_data,
    const SendUserCallback &send_callback) {
  LOG_ASSERT(IsValidMatchType(match_type));

  bool found = false;
  Uuid target_match_id;
  Json target_match_data;

  // 매치 타입과 일치하는 플레이어 수를 선택합니다.
  size_t total_players_for_match = GetNumberOfMaxPlayers(match_type);

  do {
    boost::mutex::scoped_lock lock(the_match_mutex);

    // 현재 활성화된 매치를 검사하여 플레이어가 부족한 서버를 찾습니다.
    // 순서는 UUID 를 따르므로 별다른 우선 순위가 없습니다.
    // 조금 더 공정한 절차가 필요하다면 MyMatchInfo 에 모든 플레이어 없이 게임을 진행한
    // 시간을 기록한 후, 이를 기준으로 우선순위를 부여할 수 있습니다.
    //
    // 1. 먼저 매치 타입으로 매치 ID 맵을 가져옵니다.
    auto type_itr = match_type_map.find(match_type);
    if (type_itr == match_type_map.end()) {
      break;
    }

    // 2. ID 에 해당하는 매치 목록을 순회하면서 필요한 플레이어보다 부족한
    // 서버를 찾습니다.
    auto itr = type_itr->second.begin();
    auto itr_end = type_itr->second.end();
    for (; itr != itr_end; ++itr) {
      auto match_itr = match_map.find(*itr /*match_id*/);
      LOG_ASSERT(match_itr != match_map.end());
      if (match_itr->second.players.size() < total_players_for_match) {
        // 서버를 찾았습니다.
        target_match_id = match_itr->second.match_id;
        target_match_data = match_itr->second.match_data;
        found = true;
        break;
      }
    }
  } while (false);

  if (not found) {
    // 모든 서버가 매치에 필요한 플레이어를 확보했거나 서버가 없습니다.
    // 더 이상 진행할 수 없습니다.
    LOG(INFO) << "There's no available server to send user";
    send_callback(false);
  } else {
    // 난입을 요청합니다.
    std::vector<string> account_ids { account_id };
    std::vector<Json> user_data_list { user_data };

    DedicatedServerManager::SendUsers(
        target_match_id, target_match_data, account_ids, user_data_list,
        bind(&OnUserSent, _1, _2, _3,
             target_match_data, match_type, send_callback));
  }
}


void DedicatedServerHelper::Install(
    const SessionResponseHandler &response_handler) {
  the_response_handler = response_handler;

  // 아래 4개의 콜백은 모두 매치 ID로 직렬화하여 실행됩니다.
  DedicatedServerManager::RegisterMatchResultCallback(OnMatchResultPosted);
  DedicatedServerManager::RegisterUserEnteredCallback(OnJoinedCallbackPosted);
  DedicatedServerManager::RegisterUserLeftCallback(OnLeftCallbackPosted);
  DedicatedServerManager::RegisterCustomCallback(OnCustomCallbackPosted);
}

}  // namespace dsm
