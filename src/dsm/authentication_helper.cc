// Copyright (C) 2018 iFunFactory Inc. All Rights Reserved.
//
// This work is confidential and proprietary to iFunFactory Inc. and
// must not be used, disclosed, copied, or distributed without the prior
// consent of iFunFactory Inc.

#include "authentication_helper.h"


namespace dsm {

namespace {

// 로그인 관련 JSON 키 이름 정의
const char *kAccountId = "account_id";

const char *kPlatformName = "platform";

const char *kPlatformAccessToken = "access_token";

const char *kLoggedIn = "logged_in";


void SetLogInContext(const Ptr<Session> &session,
                     const string &platform,
                     const bool logged_in) {
  Json &context = session->GetContext();
  context[kPlatformName] = platform;
  context[kLoggedIn] = logged_in;
}


void ClearLogInContext(const Ptr<Session> &session) {
  Json &context = session->GetContext();
  context.RemoveAllAttributes();
}



void OnLoggedIn(const string &account_id,
                const Ptr<Session> &session,
                bool logged_in,
                const string &platform,
                size_t try_count,
                const SessionResponseHandler &handler) {
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
          [platform, try_count, handler](const string &account_id,
                                         const Ptr<Session> &session,
                                         bool logged_out) {
            // logged_out=false 라면 다른 곳에서 로그아웃을 한 경우입니다.

            // 로그아웃 후 어떻게 처리할지 결정합니다.
            // 이 예제에서는 별다른 처리 없이 다시 로그인을 시도합니다.
            AccountManager::CheckAndSetLoggedInAsync(
                account_id, session,
                bind(&OnLoggedIn, _1, _2, _3, platform, try_count, handler));
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
      handler(ResponseResult::FAILED,
              SessionResponse(session, 500, "Internal server error.", Json()));
      return;
    }
  }  // if (not logged_in)

  // 로그인 성공
  LOG(INFO) << "Login succeed"
            << ": session_id=" << session->id()
            << ", account_id=" << account_id
            << ", platform=" << platform
            << ", try_count=" << try_count;

  // 이 정보는 서버에서 세션을 관리하기 위한 용도로만 사용하며 클라이언트로
  // 보내지 않습니다. account_id 는 AccountManager::FindLocalAccount() 함수로
  // 가져올 수 있으므로 포함하지 않습니다.
  SetLogInContext(session, platform, true /*logged in*/);

  // 클라이언트에게 보낼 응답은 이 곳에 설정합니다.
  Json response_data;
  response_data["key1"] = "value1";
  response_data["key2"] = "value2";
  response_data["key3"] = "value3";

  handler(ResponseResult::OK,
          SessionResponse(session, 200, "OK", response_data));
}


void OnLoggedOut(const string &account_id,
                 const Ptr<Session> &session,
                 bool logged_out) {
  if (not logged_out) {
    LOG(INFO) << "Not logged in: session_id=" << session->id()
              << ", account_id=" << account_id;
    return;
  }

  // 이 예제에서는 로그인 후 SetLogInContext() 로 설정한 플랫폼 정보가 있어야 합니다.
  // 만약 다음과 같은 환경이라면 assert 가 실패할 수 있습니다.
  // 1. 분산 환경에서 다른 서버가 이 정보(kPlatformName) 를 설정하지 않는 환경
  // 2. 이 서버가 다른 서버에서 로그인한 account_id 로 로그아웃을 시도할 때
  LOG_ASSERT(session->GetContext().HasAttribute(kPlatformName, Json::kString));
  const string &platform = session->GetContext()[kPlatformName].GetString();

  LOG(INFO) << "Logged out: session_id=" << session->id()
            << ", account_id=" << account_id
            << ", platform=" << platform;

  ClearLogInContext(session);
}

}  // unnamed namespace


void AuthenticationHelper::Login(
    const Ptr<Session> &session,
    const Json &message,
    const SessionResponseHandler &handler) {
  LOG_ASSERT(session);
  LOG_ASSERT(handler);

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
               << ": session=" << session->id()
               << ", message=" << message.ToString(false);
    handler(ResponseResult::FAILED,
            SessionResponse(session, 400, "Missing required fields.", Json()));
    return;
  }

  const string &account_id = message[kAccountId].GetString();
  const string &platform = message[kPlatformName].GetString();

  if (platform == "facebook") {
    // Facebook 플랫폼 사용자의 경우, 올바른 사용자인지 검증합니다.
    if (not message.HasAttribute(kPlatformAccessToken, Json::kString)) {
      LOG(ERROR) << "The message does not have '" << kPlatformAccessToken
                 << ": session=" << session->id()
                 << ", message=" << message.ToString(false);
      handler(ResponseResult::FAILED,
              SessionResponse(session, 400, "Missing required fields.", Json()));
      return;
    }

    const string &access_token = message[kPlatformAccessToken].GetString();
    FacebookAuthenticationRequest request(access_token);

    FacebookAuthenticationResponseHandler on_authenticated =
        [session, account_id, platform, handler](
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
            handler(ResponseResult::FAILED,
                    SessionResponse(session, 400, "Missing required fields.",
                                    Json()));
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
              bind(&OnLoggedIn, _1, _2, _3, platform, try_count, handler));
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
        bind(&OnLoggedIn, _1, _2, _3, platform, try_count, handler));
  }
}


void AuthenticationHelper::Logout(const Ptr<Session> &session) {

  const string &account_id = AccountManager::FindLocalAccount(session);
  if (not account_id.empty()) {
    // 이 세션으로 로그인한 적이 없습니다.
    LOG(INFO) << "This session was not used for login"
              << ": session_id=" << session->id();
    return;
  }

  // 분산 환경이라면 SetLoggedOutGlobalAsync() 함수를 사용해주세요.
  AccountManager::SetLoggedOutAsync(account_id, OnLoggedOut);
}

}  // namespace dsm