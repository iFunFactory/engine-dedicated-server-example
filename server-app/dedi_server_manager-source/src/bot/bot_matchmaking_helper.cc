// Copyright (C) 2018-2019 iFunFactory Inc. All Rights Reserved.
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


// 매치메이킹 관련 JSON 키
const char *kAccountId = "account_id";

const char *kUserData = "user_data";

const char *kMatchType = "match_type";


BotMatchmakingHelper::MatchmakingHandler the_matchmaking_handler;


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

}  // unnamed namespace


void BotMatchmakingHelper::Install(
    const MatchmakingHandler &matchmaking_handler) {
  the_matchmaking_handler = matchmaking_handler;

  funtest::Network::Register(
      kMatchThenSpawnRequest, OnMatchmakingResponseReceived);
}


void BotMatchmakingHelper::StartMatchmaking(
    const Ptr<funtest::Session> &session,
    const dsm::MatchType &match_type) {
  Json request_data, user_data;

  // 레벨 및 스코어
  // (실제 개발 환경에서는 클라이언트가 아닌 서버에서 가져온 값을 사용해야 합니다)
  user_data[dsm::kMatchLevel] = 60;
  user_data[dsm::kMMRScore] = 1000;
  // 기타 인자
  user_data["my_match_key1"] = "my_match_value1";
  user_data["my_match_key2"] = "my_match_value2";
  user_data["my_match_key3"] = "my_match_value3";

  // account_id 설정
  request_data[kAccountId] = BotAuthenticationHelper::GetAccountId(session);
  // user_data 설정
  request_data[kUserData] = user_data;
  // match_type 설정
  request_data[kMatchType] = match_type;

  LOG(INFO) << "[BOT] Request matchmaking"
            << ": bot_session_id=" << session->id()
            << ", request_data=" << request_data.ToString(false);

  session->SendMessage(kMatchThenSpawnRequest, request_data, kTcp);
}

}  // namespace bot
