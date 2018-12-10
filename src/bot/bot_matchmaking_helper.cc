// Copyright (C) 2018 iFunFactory Inc. All Rights Reserved.
//
// This work is confidential and proprietary to iFunFactory Inc. and
// must not be used, disclosed, copied, or distributed without the prior
// consent of iFunFactory Inc.

#include "bot_matchmaking_helper.h"

#include <funapi.h>
#include <funapi/test/network.h>

#include <src/bot/bot_authentication_helper.h>
#include <src/dsm/matchmaking_type.h>


namespace bot {

namespace {

const char *kMatchThenSpawnRequest = "match";

const char *kSpawnRequest = "spawn";

const char *kDediRedirect = "_sc_dedicated_server";


// 매치메이킹 관련 JSON 키
const char *kAccountId = "account_id";

const char *kUserData = "user_data";

const char *kMatchType = "match_type";


BotMatchmakingHelper::MatchmakingHandler the_matchmaking_handler;
BotMatchmakingHelper::SpawnHandler the_spawn_handler;
BotMatchmakingHelper::RedirectionHandler the_redirection_handler;


void OnMatchmakingResponseReceived(const Ptr<funtest::Session> &session,
                                   const Json &message) {
  //
  // 매치메이킹 결과를 반환합니다. 이 메시지는 dsm/matchmaking_helper.cc
  // 의 OnMatchCompleted() 함수에서 보냅니다.
  //
  // 응답 메시지 포멧은 dsm/message_handler.cc 의
  // SendMyMessage() 함수 내부를 참고하세요.
  const Json &error = message["error"];
  LOG_ASSERT(error.HasAttribute("code", Json::kInteger));
  LOG_ASSERT(error.HasAttribute("message", Json::kString));

  const int64_t err_code = error["code"].GetInteger();
  const string &err_message = error["message"].GetString();

  if (err_code != 200) {
    // 매치메이킹 요청이 실패했습니다.
    LOG(ERROR) << "Matchmaking was not successful"
               << ": message=" << err_message;
    the_matchmaking_handler(false, session);
    return;
  }

  the_matchmaking_handler(true, session);
}


void OnSpawnResponseReceived(const Ptr<funtest::Session> &session,
                             const Json &message) {
  //
  // 데디케이티드 서버 스폰 결과를 반환합니다. 이 메시지는 dsm/dedi_server_helper.cc
  // 의 OnDedicatedServerSpawned() 함수에서 보냅니다.
  //
  // 응답 메시지 포멧은 dsm/message_handler.cc 의
  // SendMyMessage() 함수 내부를 참고하세요.
  const Json &error = message["error"];
  LOG_ASSERT(error.HasAttribute("code", Json::kInteger));
  LOG_ASSERT(error.HasAttribute("message", Json::kString));

  const int64_t err_code = error["code"].GetInteger();
  const string &err_message = error["message"].GetString();

  if (err_code != 200) {
    // 데디케이티드 서버 스폰이 실패했습니다.
    LOG(ERROR) << "Spawning a dedicated server was not successful"
               << ": message=" << err_message;
    the_spawn_handler(false, session);
    return;
  }

  the_spawn_handler(true, session);
}


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

  LOG(INFO) << "Client redirection message received"
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


void BotMatchmakingHelper::Install(
    const MatchmakingHandler &matchmaking_handler,
    const SpawnHandler &spawn_handler,
    const RedirectionHandler &redirection_handler) {
  the_matchmaking_handler = matchmaking_handler;
  the_spawn_handler = spawn_handler;
  the_redirection_handler = redirection_handler;

  funtest::Network::Register(kMatchThenSpawnRequest,
                             OnMatchmakingResponseReceived);
  funtest::Network::Register(kSpawnRequest,
                             OnSpawnResponseReceived);
  funtest::Network::Register(kDediRedirect,
                             OnClientRedirection);
}


void BotMatchmakingHelper::StartMatchmaking(
    const Ptr<funtest::Session> &session) {
  Json request_data, user_data;

  // 레벨 및 스코어
  user_data[dsm::kMatchLevel] = 60;
  user_data[dsm::kRankingScore] = 1000;
  // 기타 인자
  user_data["my_match_key1"] = "my_match_value1";
  user_data["my_match_key2"] = "my_match_value2";
  user_data["my_match_key3"] = "my_match_value3";

  // account_id 설정
  request_data[kAccountId] = BotAuthenticationHelper::GetAccountId(session);
  // user_data 설정
  request_data[kUserData] = user_data;
  // match_data 설정 (dsm/matchmaking_type.h 파일에 정의된 내용)
  request_data[kMatchType] = dsm::kMatch1vs1;

  session->SendMessage(kMatchThenSpawnRequest, request_data, kTcp);
}

}  // namespace bot
