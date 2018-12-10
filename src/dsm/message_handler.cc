// Copyright (C) 2018 iFunFactory Inc. All Rights Reserved.
//
// This work is confidential and proprietary to iFunFactory Inc. and
// must not be used, disclosed, copied, or distributed without the prior
// consent of iFunFactory Inc.

#include "message_handler.h"

#include <funapi.h>
#include <functional>

#include <src/dsm/authentication_helper.h>
#include <src/dsm/session_response.h>
#include <src/dsm/dedi_server_helper.h>
#include <src/dsm/matchmaking_helper.h>
#include <src/dsm/matchmaking_server_wrapper.h>


//
// message_handler.cc 코드
//
// 이 파일은 클라이언트로부터 오는 모든 요청을 처리하는 곳입니다.
//
// RegisterMessageHandler() 안에서 등록한 메시지 타입(kLoginRequest 등)과
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

const char *kLoginRequest = "login";

const char *kSpawnRequest = "spawn";

const char *kMatchThenSpawnRequest = "match";

const char *kCancelMatchRequest = "cancel_match";


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


void OnSessionOpened(const Ptr<Session> &session) {
  LOG(INFO) << "Session opened: session=" << session->id();
}


void OnSessionClosed(const Ptr<Session> &session, SessionCloseReason reason) {
  LOG(INFO) << "Session closed: session=" << session->id()
            << ", reason=" << reason;

  // 세션 연결이 어떤 이유에서 닫혔습니다. 이 세션이 로그인을 했다면
  // 다음에 다시 로그인할 수 있도록 로그아웃 처리를 해야 합니다.
  // 이후 과정은 authentication_helper.cc 를 참고하세요.
  AuthenticationHelper::Logout(session);
}


void OnLoginRequest(const Ptr<Session> &session, const Json &message) {
  const SessionId &session_id = session->id();
  const Json &session_context = session->GetContext();

  SessionResponseHandler response_handler =
      [](const ResponseResult error, const SessionResponse &response) {
        LOG_ASSERT(response.session);
        SendMyMessage(response.session, kLoginRequest, response.error_code,
                      response.error_message, response.data);
      };

  LOG(INFO) << "OnLoginRequest"
            << ": session=" << session_id
            << ", context=" << session_context.ToString(false)
            << ", message=" << message.ToString(false);

  // 이후 과정은 authentication_helper.cc 를 참고하세요.
  AuthenticationHelper::Login(session, message, response_handler);
}


void OnSpawnRequest(const Ptr<Session> &session, const Json &message) {
  const SessionId &session_id = session->id();
  const Json &session_context = session->GetContext();

  SessionResponseHandler response_handler =
      [](const ResponseResult error, const SessionResponse &response) {
        if (not response.session) {
          // response.session 이 Session::kNullPtr 이어서 메시지를 보낼 수 없습니다.
          // 자세한 내용은 dedi_server_helper.cc 의
          // OnDedicatedServerSpawned() 함수를 참고하세요.
          return;
        }

        SendMyMessage(response.session, kSpawnRequest, response.error_code,
                      response.error_message, response.data);
      };

  LOG(INFO) << "OnSpawnRequest"
            << ": session=" << session_id
            << ", context=" << session_context.ToString(false)
            << ", message=" << message.ToString(false);

  // 이후 과정은 dedi_server_helper.cc 를 참고하세요.
  DediServerHelper::ProcessDediServerSpawn1(session, message, response_handler);
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
        SendMyMessage(response.session, kMatchThenSpawnRequest,
                      response.error_code, response.error_message,
                      response.data);
      };

  // 이후 과정은 matchmaking_helper.cc 를 참고하세요.
  MatchmakingHelper::ProcessMatchmaking(session, message, response_handler);
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
        SendMyMessage(response.session, kCancelMatchRequest, response.error_code,
                      response.error_message, response.data);
      };

  // 이후 과정은 matchmaking_helper.cc 를 참고하세요.
  MatchmakingHelper::CancelMatchmaking(session, message, response_handler);
}

}  // unnamed namespace


void RegisterMessageHandler() {
  // 세션 열림 및 닫힘 핸들러를 등록합니다.
  HandlerRegistry::Install2(OnSessionOpened, OnSessionClosed);
  // 로그인 요청 핸들러를 등록합니다.
  HandlerRegistry::Register(kLoginRequest, OnLoginRequest);
  // 데디케이티드 서버 스폰(Spawn) 요청 핸들러를 등록합니다.
  HandlerRegistry::Register(kSpawnRequest, OnSpawnRequest);
  // 매치메이킹 후 매치가 성사된 유저들을 모아 데디케이티드 서버를 스폰합니다.
  HandlerRegistry::Register(kMatchThenSpawnRequest, OnMatchThenSpawnRequest);
  // 매치메이킹 요청을 취소합니다.
  HandlerRegistry::Register(kCancelMatchRequest, OnCancelMatchRequest);

  // 매치메이킹 후 데디케이티드 서버 생성이 완료된 클라이언트로
  // 스폰 결과에 대한 응답을 보낼 때 사용합니다.
  // ( 클라이언트 리다이렉션 메시지는 엔진 내부에서 이 핸들러와 별개로 처리합니다. )
  SessionResponseHandler response_handler =
      [](const ResponseResult error, const SessionResponse &response) {
        LOG_ASSERT(response.session);
        SendMyMessage(response.session, kMatchThenSpawnRequest,
                      response.error_code, response.error_message, response.data);
      };

  MatchmakingServerWrapper::Install(response_handler);
}

}  // namespace dsm