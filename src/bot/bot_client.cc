// Copyright (C) 2018 iFunFactory Inc. All Rights Reserved.
//
// This work is confidential and proprietary to iFunFactory Inc. and
// must not be used, disclosed, copied, or distributed without the prior
// consent of iFunFactory Inc.

#include "bot_client.h"

#include <funapi.h>
#include <funapi/test/network.h>
#include <gflags/gflags_declare.h>
#include <funapi/common/json.h>

#include <src/bot/bot_authentication_helper.h>
#include <src/bot/bot_dedi_server_spawn_helper.h>

DEFINE_string(dsm_server_host, "127.0.0.1",
              "Dedicated server manager server host");
DEFINE_uint64(dsm_server_port, 8012, "Dedicated server manager server port");

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


void OnSpawnResponseReceived(const bool succeed,
                             const Ptr<funtest::Session> &session) {
  Json &context = session->GetContext();
  LOG_ASSERT(context.HasAttribute(kClientIndex, Json::kInteger));
  const int64_t index = context[kClientIndex].GetInteger();


  LOG(INFO) << "Spawn response received: session_id=" << session->id()
            << ", index=" << index
            << ", spawned=" << succeed;

  if (not succeed) {
    // 데디케이티드 서버 스폰 요청이 실패했습니다.
    // OnClientRedirection() 호출 없이 끝나므로 여기서 재시도 할지,
    // 사용자에게 실패를 알릴 지 결정해야 합니다.
  }

  // 데디케이티드 서버 스폰 요청에 성공했습니다.
  // 이후 곧바로 OnClientRedirection() 함수를 호출합니다.
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

  // 로그인에 성공했습니다. 데디케이티드 서버 요청을 진행합니다.
  BotDediServerSpawnHelper::Spawn(session);
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


void BotClient::Install(int threads, int bot_clients) {
  // 봇 클라이언트의 세션 핸들러를 등록합니다.
  funtest::Network::Install(OnSessionOpened, OnSessionClosed, threads);

  BotAuthenticationHelper::Install(OnLoginResponseReceived);
  BotDediServerSpawnHelper::Install(
      OnSpawnResponseReceived, OnClientRedirection);
  the_bot_clients = bot_clients;
}


void BotClient::Uninstall() {
  the_bot_clients = 0;
}


void BotClient::Start() {
  LOG_ASSERT(the_bot_clients != 0);

  for (int index = 0; index < the_bot_clients; ++index) {
    LOG(INFO) << "Connecting: index=" << index
              << ", host=" << FLAGS_dsm_server_host
              << ", port=" << FLAGS_dsm_server_port;
    Ptr<funtest::Session> session = funtest::Session::Create();

    // 연결 후 세션 인덱스를 식별하기 위해 세션 컨텍스트에 저장합니다.
    Json &context = session->GetContext();
    context[kClientIndex] = index;

    session->ConnectTcp(
        FLAGS_dsm_server_host, FLAGS_dsm_server_port, kJsonEncoding);
  }
}

}  // namespace bot