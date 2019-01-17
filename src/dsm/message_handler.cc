// Copyright (C) 2018-2019 iFunFactory Inc. All Rights Reserved.
//
// This work is confidential and proprietary to iFunFactory Inc. and
// must not be used, disclosed, copied, or distributed without the prior
// consent of iFunFactory Inc.

#include "message_handler.h"

#include <funapi.h>
#include <functional>

#include <src/dsm/authentication_helper.h>
#include <src/dsm/session_response.h>
#include <src/dsm/dedicated_server_helper.h>
#include <src/dsm/matchmaking_helper.h>
#include <src/dsm/matchmaking_server_wrapper.h>


//
// message_handler.cc 코드
//
// 이 파일은 클라이언트로부터 오는 모든 요청을 처리하는 곳입니다.
//
// RegisterMessageHandler() 안에서 등록한 메시지 타입(kLoginMessage 등)과
// 일치하는 요청이 왔을 때 이 곳에서 처리할 수 있습니다.
// 만약 등록하지 않은 메시지 타입으로 요청이 올 경우 엔진 내부적으로 에러 메시지를
// 출력합니다.
//
// 참고사항
//
// 1. 이 예제에서 세션과 관련있는 로그를 기록할 떄는 사용자를 추적할 수 있도록
// 항상 session ID 를 함께 출력합니다..
//
// 2. 클라이언트에 보낼 모든 메시지를 SendMyMessage() 안에서 처리하고 있습니다.
// 한 함수 안에서 응답을 처리하면 메시지를 일관성 있게 정의할 수 있습니다.
//

namespace dsm {

namespace {

const char *kLoginMessage = "login";

const char *kLogoutMessage = "logout";

const char *kMatchThenSpawnMessage = "match";

const char *kCancelMatchMessage = "cancel_match";


void SendMyMessage(const Ptr<Session> &session,
                   const string &message_type,
                   const int64_t code,
                   const string &message,
                   const Json &data) {
  //
  // 클라이언트에게 보낼 실패 응답을 이 곳에 설정합니다.
  //

  // 로직 상 NULL 세션 값이 오지 않도록 assert 를 추가했습니다.
  LOG_ASSERT(session);

  // message 변수를 검사하지 않기 때문에 클라이언트는 빈 message 문자열을 받을 수
  // 있습니다. 만약 빈 문자열을 허용하고 싶지 않다면 아래 assert 를 추가해주세요.
  // LOG_ASSERT(not message.empty());

  Json response;
  response["error"]["code"] = code;
  response["error"]["message"] = message;
  if (data.IsNull()) {
    // empty object
    response["data"] = "{}";
  } else {
    // data 는 반드시 object 형태만 포함합니다. value (문자열, 정수 등) 또는
    // 배열 형태의 JSON 데이터를 허용하지 않게 합니다.
    LOG_ASSERT(data.IsObject());
    response["data"] = data;
  }

  // 서버가 2개 이상(TCP, HTTP)의 프로토콜을 허용한다면 메시지를 보낼 때
  // 반드시 kTcp 로 프로토콜을 지정해야 합니다. 그렇지 않을 경우
  // 모호한(Ambiguous) 프로토콜로 간주하고 메시지를 보내지 않습니다.
  session->SendMessage(message_type, response, kDefaultEncryption, kTcp);
}


void OnLogin(const ResponseResult error, const SessionResponse &response) {
  // 로그인 이후 처리를 담당합니다.
  LOG_ASSERT(response.session);

  LOG(INFO) << "OnLogin: session=" << to_string(response.session->id())
            << ", result=" << (error == ResponseResult::OK ? "ok" : "failed")
            << ", data=" << response.data.ToString(false);

  SendMyMessage(response.session, kLoginMessage, response.error_code,
                response.error_message, response.data);
}


void OnLogout(const ResponseResult error,
              const SessionResponse &response,
              bool caused_by_session_close) {
  // 로그아웃 이후 처리를 담당합니다.
  LOG_ASSERT(response.session);

  LOG(INFO) << "OnLogout: session=" << to_string(response.session->id())
            << ", result=" << (error == ResponseResult::OK ? "ok" : "failed")
            << ", caused_by="
            << (caused_by_session_close ? "session close" : "action")
            << ", data=" << response.data.ToString(false);

  Event::Invoke([response, caused_by_session_close]() {
    // 매칭 요청을 취소하지 않고 로그아웃/세션 종료를 할 수 있습니다.
    // 이전에 이 세션으로 매칭을 요청한 기록이 있는지 확인 후 취소합니다.
    MatchmakingHelper::CancelMatchmaking(response.session);

    // 세션 닫힘 이벤트가 아니라면 로그아웃 요청으로 이 콜백을 호출한 경우므로
    // 메시지를 보냅니다.
    if (not caused_by_session_close) {
      SendMyMessage(response.session, kLogoutMessage, response.error_code,
                    response.error_message, response.data);
    }
  }, response.session->id());
}


void OnSessionOpened(const Ptr<Session> &session) {
  LOG(INFO) << "Session opened: session=" << session->id();
}


void OnSessionClosed(const Ptr<Session> &session, SessionCloseReason reason) {
  LOG(INFO) << "Session closed: session_id=" << session->id()
            << ", reason=" << reason;

  // 세션 연결이 닫혔습니다. 만약 이 세션이 로그인을 했다면
  // 정상적인 흐름으로 다시 로그인할 수 있도록 로그아웃 처리를 하는 게 좋습니다.
  AuthenticationHelper::Logout(session,
      bind(&OnLogout, _1, _2, true /* caused by session close */));
}


void OnTcpTransportDetached(const Ptr<Session> &session) {
  LOG(INFO) << "TCP transport detached: session_id=" << session->id();
  // TCP 연결이 닫혔으나 세션은 유효합니다. 이 세션이 매칭 요청을 했다면
  // 매칭을 취소합니다. 클라이언트는 TCP 연결이 끊어졌을 때 매칭을 다시 시도하게
  // 해야 합니다.
  Event::Invoke([session]() {
    MatchmakingHelper::CancelMatchmaking(session);
  }, session->id());
}


void OnLoginRequest(const Ptr<Session> &session, const Json &message) {
  const SessionId &session_id = session->id();
  const Json &session_context = session->GetContext();

  LOG(INFO) << "OnLoginRequest"
            << ": session=" << session_id
            << ", context=" << session_context.ToString(false)
            << ", message=" << message.ToString(false);

  // 로그인을 시도합니다.
  // OnLogin 함수는 이 세션이 로그인을 성공/실패한 결과를 보낼 때 사용합니다.
  // OnLogout 함수는 중복 로그인 처리(로그인한 다른 세션을 로그아웃) 시 사용합니다.
  // 이후 과정은 authentication_helper.cc 를 참고하세요.
  AuthenticationHelper::Login(session, message, OnLogin,
      bind(&OnLogout, _1, _2, false /* not caused by session close */));
}


void OnLogoutRequest(const Ptr<Session> &session, const Json &message) {
  const SessionId &session_id = session->id();
  const Json &session_context = session->GetContext();

  LOG(INFO) << "OnLogoutRequest"
            << ": session=" << session_id
            << ", context=" << session_context.ToString(false)
            << ", message=" << message.ToString(false);

  // 이후 과정은 authentication_helper.cc 를 참고하세요.
  AuthenticationHelper::Logout(session,
      bind(&OnLogout, _1, _2, false /* not caused by session close */));
}


void OnDedicatedServerSpawned(
    const ResponseResult error, const SessionResponse &response) {
  // 클라이언트 리다이렉션 메시지는 엔진 내부에서 이 핸들러와 별개로 처리합니다.
  // 이 곳에서는 서버 생성 결과만 메시지를 보냅니다.
  LOG_ASSERT(response.session);

  LOG(INFO) << "OnDedicatedServerSpawned"
            << ": session_id=" << response.session->id()
            << ", error=" << (error == ResponseResult::OK ? "ok" : "failed")
            << ", response.error_code=" << response.error_code
            << ", response.error_message=" << response.error_message
            << ", response.data=" << response.data.ToString(false);

  SendMyMessage(response.session, kMatchThenSpawnMessage,
                response.error_code, response.error_message,
                response.data);
}


void OnMatchThenSpawnRequest(const Ptr<Session> &session, const Json &message) {
  const SessionId &session_id = session->id();
  const Json &session_context = session->GetContext();

  LOG(INFO) << " OnMatchThenSpawnRequest"
            << ": session=" << session_id
            << ", context=" << session_context.ToString(false)
            << ", message=" << message.ToString(false);

  SessionResponseHandler response_handler =
      [](const ResponseResult error, const SessionResponse &response) {
        LOG_ASSERT(response.session);
        SendMyMessage(response.session, kMatchThenSpawnMessage,
                      response.error_code, response.error_message,
                      response.data);
      };

  // 이후 과정은 matchmaking_helper.cc 를 참고하세요.
  MatchmakingHelper::ProcessSpawnOrMatchmaking(
      session, message, response_handler);
}


void OnCancelMatchRequest(const Ptr<Session> &session, const Json &message) {
  const SessionId &session_id = session->id();
  const Json &session_context = session->GetContext();

  LOG(INFO) << " OnCancelMatchRequest"
            << ": session=" << session_id
            << ", context=" << session_context.ToString(false)
            << ", message=" << message.ToString(false);

  SessionResponseHandler response_handler =
      [](const ResponseResult error, const SessionResponse &response) {
        LOG_ASSERT(response.session);
        SendMyMessage(response.session, kCancelMatchMessage, response.error_code,
                      response.error_message, response.data);
      };

  // 이후 과정은 matchmaking_helper.cc 를 참고하세요.
  MatchmakingHelper::CancelMatchmaking(session, message, response_handler);
}

}  // unnamed namespace


void RegisterMessageHandler() {
  // 세션 열림 및 닫힘 핸들러를 등록합니다.
  HandlerRegistry::Install2(OnSessionOpened, OnSessionClosed);
  // TCP 연결이 끊어졌을 때 호출할 핸들러를 등록합니다. 세션이 닫힌 건 아니므로
  // 언제든지 다른 트랜스포트를 통해 이 세션을 사용할 수 있습니다.
  // (WIFI -> LTE 이동과 같은 상황이 좋은 예입니다)
  HandlerRegistry::RegisterTcpTransportDetachedHandler(OnTcpTransportDetached);
  // 로그인 요청 핸들러를 등록합니다.
  HandlerRegistry::Register(kLoginMessage, OnLoginRequest);
  // 로그아웃 요청 핸들러를 등록합니다
  HandlerRegistry::Register(kLogoutMessage, OnLogoutRequest);
  // 매치메이킹 후 매치가 성사된 유저들을 모아 데디케이티드 서버를 스폰합니다.
  HandlerRegistry::Register(kMatchThenSpawnMessage, OnMatchThenSpawnRequest);
  // 매치메이킹 요청을 취소합니다.
  HandlerRegistry::Register(kCancelMatchMessage, OnCancelMatchRequest);

  // 데디케이티드 서버 생성이 완료된 클라이언트로
  // 스폰 결과에 대한 응답을 보낼 때 사용합니다.
  dsm::DedicatedServerHelper::Install(OnDedicatedServerSpawned);
  // 매치매이킹 서버가 내부적으로 쓰는 핸들러들을 등록합니다.
  dsm::MatchmakingServerWrapper::Install();
}

}  // namespace dsm
