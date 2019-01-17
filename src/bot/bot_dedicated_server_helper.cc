// Copyright (C) 2018-2019 iFunFactory Inc. All Rights Reserved.
//
// This work is confidential and proprietary to iFunFactory Inc. and
// must not be used, disclosed, copied, or distributed without the prior
// consent of iFunFactory Inc.

#include "bot_dedicated_server_helper.h"

#include <funapi.h>
#include <funapi/test/network.h>

#include <src/bot/bot_authentication_helper.h>


namespace bot {

namespace {


const char *kDediRedirect = "_sc_dedicated_server";


BotDedicatedServerHelper::RedirectionHandler the_redirection_handler;


void OnClientRedirection(const Ptr<funtest::Session> &session,
                         const Json &message) {

  // 데디케이티드 서버 스폰이 성공했고 클라이언트가 데디케이티드 서버로 접속할 차례입니다.
  // 데디케이티드 서버 매니저는 클라이언트로 다음 메시지를 전송합니다.
  //
  // {
  //   "redirect":{
  //     "host":"IP 주소 또는 URL 주소",
  //     "port":5000,
  //     "token":"2c777dae26b1cc310c04f984ec5e8ede"
  //   }
  // }
  // host: 데디케이티드 서버 접속 주소입니다.
  // port: 데디케이티드 서버 포트 번호입니다.
  // token: 데디케이티드 서버가 신뢰하는 클라이언트인지 확인하는 데 사용할 토큰입니다.

  LOG(INFO) << "[BOT] Received client redirection"
            << ": session_id=" << session->id()
            << ", message=" << message.ToString(false);

  LOG_ASSERT(message.HasAttribute("redirect", Json::kObject));

  const Json &redirect = message["redirect"];
  LOG_ASSERT(redirect.HasAttribute("host", Json::kString));
  LOG_ASSERT(redirect.HasAttribute("port", Json::kInteger));
  LOG_ASSERT(redirect.HasAttribute("token", Json::kString));

  const string &host = redirect["host"].GetString();
  const int64_t port = redirect["port"].GetInteger();
  const string &token = redirect["token"].GetString();

  the_redirection_handler(session, host, port, token);
}

}  // unnamed namespace


void BotDedicatedServerHelper::Install(
    const RedirectionHandler &redirection_handler) {
  the_redirection_handler = redirection_handler;

  funtest::Network::Register(kDediRedirect, OnClientRedirection);
}

}  // namespace bot
