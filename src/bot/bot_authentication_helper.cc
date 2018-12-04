// Copyright (C) 2018 iFunFactory Inc. All Rights Reserved.
//
// This work is confidential and proprietary to iFunFactory Inc. and
// must not be used, disclosed, copied, or distributed without the prior
// consent of iFunFactory Inc.

#include "bot_authentication_helper.h"


namespace bot {

namespace {

const char *kLoginRequest = "login";

// 로그인 관련 JSON 키 이름 정의
const char *kAccountId = "account_id";

const char *kPlatformName = "platform";

const char *kPlatformAccessToken = "access_token";

const char *kLoggedIn = "logged_in";

BotAuthenticationHelper::LoginHandler the_login_handler;

void OnLoginResponseReceived(
    const Ptr<funtest::Session> &session,
    const Json &message) {
  LOG(INFO) << "Login response received: session_id=" << session->id()
            << ", message=" << message.ToString(false);

  // 응답 메시지 포멧은 dsm/message_handler.cc 의
  // SendMyMessage() 함수 내부를 참고하세요.
  const Json &error = message["error"];
  LOG_ASSERT(error.HasAttribute("code", Json::kInteger));
  LOG_ASSERT(error.HasAttribute("message", Json::kString));

  const int64_t err_code = error["code"].GetInteger();
  const string &err_message = error["message"].GetString();

  bool logged_in = err_code == 200;

  LOG_ASSERT(session);
  Json &context = session->GetContext();
  context[kLoggedIn] = logged_in;

  if (not logged_in) {
    // 로그인에 실패했습니다.
    LOG(ERROR) << "Error occurred while requesting login"
               << ": message=" << err_message;
    the_login_handler(false, session);
    return;
  }

  the_login_handler(true, session);
}

}  // unnamed namespace


void BotAuthenticationHelper::Install(const LoginHandler &login_handler) {
  LOG_ASSERT(login_handler);
  the_login_handler = login_handler;
  funtest::Network::Register(kLoginRequest, OnLoginResponseReceived);
}


void BotAuthenticationHelper::Login(const Ptr<funtest::Session> &session) {
  LOG_ASSERT(session);
  Json &context = session->GetContext();


  // 계정을 생성합니다. 이 예제에서는 매 로그인 시 랜덤으로 생성한 계정 ID를 사용합니다.
  // 실제 클라이언트 코드를 작성할 때는 게스트 ID 를 생성한 후 디바이스 또는 기기에 저장
  // 하거나 Facebook, Google 등 외부 플랫폼 SDK 로부터 가져온 ID를 써야 합니다.
  const string account_id = RandomGenerator::GenerateAlphanumeric(10, 10);

  // 플랫폼을 설정합니다. facebook, google, steam 등을 이 곳 설정하고,
  // 서버에서는 이 정보를 통해 플랫폼 별 인증 방식을 고를 수 있습니다.
  const string platform = "guest";

  // 플랫폼에서 요구하는 정보(ex 액세스 토큰) 를 이 곳, 또는 별도로 추가합니다.
  const string access_token = "your access token";

  // 봇 클라이언트 인터페이스는 세션을 기준으로 하기 때문에 로그인에 사용하는 정보들은
  // 추후 세션 ID 또는 세션으로 찾을 수 있어야 합니다.
  // 이 예제에서는 세션 컨텍스트에 이 정보들을 저장합니다.
  context[kAccountId] = account_id;
  context[kPlatformName] = platform;
  context[kPlatformAccessToken] = access_token;

  // 로그인 요청 데이터를 가공합니다.
  // 서버의 로그인 메시지 처리는 dsm/authentication_helper.cc 에서 확인해주세요.
  Json request_data;
  request_data[kAccountId] = account_id;
  request_data[kPlatformName] = platform;
  request_data[kPlatformAccessToken] = "your access token";

  // 로그인 요청 메시지를 보냅니다. 서버는 이 메시지를 처리한 후 같은 메시지 타입으로
  // 응답을 보냅니다. 봇 클라이언트는 서버에서 보낸 메시지를 OnLoginResponseReceived()
  // 핸들러를 통해 응답 메시지를 받습니다.
  session->SendMessage(kLoginRequest, request_data, kTcp);
}


string BotAuthenticationHelper::GetAccountId(
    const Ptr<funtest::Session> &session) {
  LOG_ASSERT(session);
  Json &context = session->GetContext();

  if (not context.HasAttribute(kAccountId, Json::kString)) {
    return "";
  }

  return context[kAccountId].GetString();
}


bool BotAuthenticationHelper::IsLoggedIn(
    const Ptr<funtest::Session> &session) {
  LOG_ASSERT(session);
  Json &context = session->GetContext();

  if (not context.HasAttribute(kLoggedIn, Json::kBoolean)) {
    return false;
  }

  return context[kLoggedIn].GetBool();
}

}  // namespace bot