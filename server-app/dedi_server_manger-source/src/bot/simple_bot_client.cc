// Copyright (C) 2018-2019 iFunFactory Inc. All Rights Reserved.
//
// This work is confidential and proprietary to iFunFactory Inc. and
// must not be used, disclosed, copied, or distributed without the prior
// consent of iFunFactory Inc.

#include "simple_bot_client.h"

#include <funapi.h>
#include <funapi/test/network.h>
#include <gflags/gflags_declare.h>
#include <funapi/common/json.h>

#include <src/bot/bot_authentication_helper.h>
#include <src/bot/bot_dedicated_server_helper.h>
#include <src/bot/bot_matchmaking_helper.h>
#include <src/dsm/matchmaking_type.h>


DEFINE_string(dsm_server_host, "127.0.0.1",
              "Dedicated server manager server host.");

DEFINE_uint64(dsm_server_port, 8012, "Dedicated server manager server port.");

//
// 봇 클라이언트 코드입니다. 서버에서 사용하는 Session 과 비슷한 funtest::Session
// 을 사용하지만, 봇과 서버 세션은 서로 다른 세션 객체, 세션 ID 를 사용하고 저장하는
// 내용(Session 컨텍스트) 등 또한 다르다는 점을 주의하기 바랍니다.
// 또한 이 클래스는 서버에서 스트레스 테스트를 목적으로 제공하므로 실제 유니티/언리얼
// 플러그인에서 제공하는 기능들과 다를 수 있습니다.
//

namespace bot {

namespace {

const char *kClientIndex = "index";

int the_bot_clients = 0;

dsm::MatchType the_match_type = dsm::kNoMatching;

void OnMatchmakingResponseReceived(const bool succeed,
                                   const Ptr<funtest::Session> &session) {
  Json &context = session->GetContext();
  LOG_ASSERT(context.HasAttribute(kClientIndex, Json::kInteger));
  const int64_t index = context[kClientIndex].GetInteger();

  LOG(INFO) << "Received a matchmaking response"
            << ": session_id=" << session->id()
            << ", index=" << index
            << ", succeed=" << succeed;

  if (not succeed) {
    // 매치메이킹 요청이 실패했습니다.
    // OnSpawnResponseReceived() 호출 없이 끝나므로
    // 1. 여기서 재시도 할지,
    // 2. 사용자에게 실패를 알릴 지 결정해야 합니다.
  }

  // 매치메이킹 요청에 성공했습니다.
  // 몇 분 이내로 데디케이티드 서버 스폰 요청이 성공하고
  // OnSpawnResponseReceived() 를 호출할 것입니다.
}


void OnClientRedirection(
    const Ptr<funtest::Session> &session,
    const string &host,
    const int64_t port,
    const string &token) {
  Json &context = session->GetContext();
  LOG_ASSERT(context.HasAttribute(kClientIndex, Json::kInteger));
  const int64_t index = context[kClientIndex].GetInteger();

  bool logged_id = BotAuthenticationHelper::IsLoggedIn(session);
  const string account_id = BotAuthenticationHelper::GetAccountId(session);

  if (not logged_id) {
    // 스폰 요청 후 리다이렉션 메시지를 받기 전에 로그아웃을 했습니다.
    // 더 이상 진행할 필요가 없습니다.
    return;
  }

  // TODO: 실제 클라이언트 코드를 작성할 때는 이 곳에서 데디케이티드 서버
  // 접속을 진행하면 됩니다.
  LOG(INFO) << "Switch to spawned dedicated server"
            << ": session_id=" << session->id()
            << ", index=" << index
            << ", host=" << host
            << ", port=" << port
            << ", token=" << token;
}


void OnLoginResponseReceived(
    const bool succeed,
    const Ptr<funtest::Session> &session) {
  if (not succeed) {
    LOG(INFO) << "Login failed: session_id=" << session->id();
    return;
  }


  // 로그인에 성공했습니다. 매치메이킹 + 데디케이티드 서버 요청을 진행합니다.
  // dsm::kNoMatching = 매칭 없이 진행, dsm/matchmaking_type.h 파일에 정의된 내용
  BotMatchmakingHelper::StartMatchmaking(session, the_match_type);
}


void OnSessionOpened(const Ptr<funtest::Session> &session) {
  LOG(INFO) << "[BOT] Session opened"
            << ": bot_session_id=" << session->id();

  // 세션 연결을 맺는 데 성공했습니다 이제 임시 계정 정보를 만든 후 로그인을 시도합니다.
  BotAuthenticationHelper::Login(session);
}


void OnSessionClosed(const Ptr<funtest::Session> &session,
                     SessionCloseReason reason) {
  LOG(INFO) << "[BOT] Session closed: session=" << session->id()
            << ", reason=" << reason;
}

}  // unnamed namespace


void SimpleBotClient::Install(
    int threads, int bot_clients, const dsm::MatchType match_type) {
  // 봇 클라이언트의 세션 핸들러를 등록합니다.
  funtest::Network::Option option(threads);
  funtest::Network::Install(OnSessionOpened, OnSessionClosed, option);

  BotAuthenticationHelper::Install(OnLoginResponseReceived);

  BotMatchmakingHelper::Install(OnMatchmakingResponseReceived);

  BotDedicatedServerHelper::Install(OnClientRedirection);

  LOG_ASSERT(dsm::IsValidMatchType(match_type));
  the_match_type = match_type;

  the_bot_clients = bot_clients;
}


void SimpleBotClient::Uninstall() {
  the_bot_clients = 0;
}


void SimpleBotClient::Start() {
  LOG_ASSERT(the_bot_clients != 0);

#if 0
// 봇을 한 번에 모두 실행합니다.
for (int index = 0; index < the_bot_clients; ++index) {
  LOG(INFO) << "[BOT] Connecting: index=" << index
            << ", host=" << FLAGS_dsm_server_host
            << ", port=" << FLAGS_dsm_server_port;
  Ptr<funtest::Session> session = funtest::Session::Create();

  // 연결 후 세션 인덱스를 식별하기 위해 세션 컨텍스트에 저장합니다.
  Json &context = session->GetContext();
  context[kClientIndex] = index;

  session->ConnectTcp(
      FLAGS_dsm_server_host, FLAGS_dsm_server_port, kJsonEncoding);
}
#else
// 봇을 2초 간격으로 실행합니다.
for (int index = 0; index < the_bot_clients; ++index) {
  Timer::ExpireAfter(
      WallClock::FromSec(index*2),
      [index](const Timer::Id &, const WallClock::Value &) {
        LOG(INFO) << "[BOT] Connecting: index=" << index
                  << ", host=" << FLAGS_dsm_server_host
                  << ", port=" << FLAGS_dsm_server_port;
        Ptr<funtest::Session> session = funtest::Session::Create();

        // 연결 후 세션 인덱스를 식별하기 위해 세션 컨텍스트에 저장합니다.
        Json &context = session->GetContext();
        context[kClientIndex] = index;

        session->ConnectTcp(
            FLAGS_dsm_server_host, FLAGS_dsm_server_port, kJsonEncoding);
      });
}
#endif
}

}  // namespace bot
