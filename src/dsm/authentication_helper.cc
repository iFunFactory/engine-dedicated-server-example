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


void OnLoggedIn(const string &account_id,
                const Ptr<Session> &session,
                bool logged_in,
                const string &platform,
                size_t try_count,
                const SessionResponseHandler &response_handler) {
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
          [platform, try_count, response_handler](const string &account_id,
                                                  const Ptr<Session> &session,
                                                  bool logged_out) {
            // logged_out=false 라면 다른 곳에서 로그아웃을 한 경우입니다.

            // 로그아웃 후 어떻게 처리할지 결정합니다.
            // 이 예제에서는 별다른 처리 없이 다시 로그인을 시도합니다.
            AccountManager::CheckAndSetLoggedInAsync(
                account_id, session,
                bind(&OnLoggedIn, _1, _2, _3,
                     platform, try_count, response_handler));
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
      response_handler(
          ResponseResult::FAILED,
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

  // 클라이언트에게 보낼 응답을 이 곳에 설정합니다.
  Json response_data;
  response_data["key1"] = "value1";
  response_data["key2"] = "value2";
  response_data["key3"] = "value3";

  response_handler(
      ResponseResult::FAILED,
      SessionResponse(session, 200, "OK", response_data));
}

}  // unnamed namespace


void AuthenticationHelper::ProcessAuthentication(
    const Ptr<Session> &session,
    const Json &message,
    const SessionResponseHandler &response_handler) {
  LOG_ASSERT(session);
  LOG_ASSERT(response_handler);

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
    response_handler(
        ResponseResult::FAILED,
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
      response_handler(
          ResponseResult::FAILED,
          SessionResponse(session, 400, "Missing required fields.", Json()));
      return;
    }

    const string &access_token = message[kPlatformAccessToken].GetString();
    FacebookAuthenticationRequest request(access_token);

    FacebookAuthenticationResponseHandler on_authenticated =
        [session, account_id, platform, response_handler](
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
            response_handler(
                ResponseResult::FAILED,
                SessionResponse(
                    session, 400, "Missing required fields.", Json()));
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
              bind(&OnLoggedIn, _1, _2, _3,
                   platform, try_count, response_handler));
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
        bind(&OnLoggedIn, _1, _2, _3,
             platform, try_count, response_handler));
  }
}

}  // namespace dsm