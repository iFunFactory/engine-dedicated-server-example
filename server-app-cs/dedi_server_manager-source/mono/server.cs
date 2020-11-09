// Copyright (C) 2020 iFunFactory Inc. All Rights Reserved.
//
// This work is confidential and proprietary to iFunFactory Inc. and
// must not be used, disclosed, copied, or distributed without the prior
// consent of iFunFactory Inc.

using funapi;
using Newtonsoft.Json.Linq;
using System;
using System.Collections.Generic;

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

namespace DediServerManager
{
	public class Server
	{
		const string kLoginMessage = "login";
		const string kLogoutMessage = "logout";
		const string kMatchThenSpawnMessage = "match";
		const string kCancelMatchMessage = "cancel_match";

		public static bool Install(ArgumentMap arguments)
		{
			// 세션 열림 및 닫힘 핸들러를 등록합니다.
			NetworkHandlerRegistry.RegisterSessionHandler (
				new NetworkHandlerRegistry.SessionOpenedHandler (OnSessionOpened),
				new NetworkHandlerRegistry.SessionClosedHandler (OnSessionClosed));
			// TCP 연결이 끊어졌을 때 호출할 핸들러를 등록합니다. 세션이 닫힌 건 아니므로
			// 언제든지 다른 트랜스포트를 통해 이 세션을 사용할 수 있습니다.
			// (WIFI -> LTE 이동과 같은 상황이 좋은 예입니다)
			NetworkHandlerRegistry.RegisterTcpTransportDetachedHandler(OnTcpTransportDetached);
			// 로그인 요청 핸들러를 등록합니다.
			NetworkHandlerRegistry.RegisterMessageHandler (
					kLoginMessage, new NetworkHandlerRegistry.JsonMessageHandler (OnLoginRequest));
			// 로그아웃 요청 핸들러를 등록합니다
			NetworkHandlerRegistry.RegisterMessageHandler (
					kLogoutMessage, new NetworkHandlerRegistry.JsonMessageHandler (OnLogoutRequest));
			// 매치메이킹 후 매치가 성사된 유저들을 모아 데디케이티드 서버를 스폰합니다.
			NetworkHandlerRegistry.RegisterMessageHandler (
					kMatchThenSpawnMessage, new NetworkHandlerRegistry.JsonMessageHandler (OnMatchThenSpawnRequest));
			// 매치메이킹 요청을 취소합니다.
			NetworkHandlerRegistry.RegisterMessageHandler (
					kCancelMatchMessage, new NetworkHandlerRegistry.JsonMessageHandler (OnCancelMatchRequest));

			// 데디케이티드 서버 생성이 완료된 클라이언트로
			// 스폰 결과에 대한 응답을 보낼 때 사용합니다.
			DedicatedServerHelper.Install(OnDedicatedServerSpawned);
			// 매치매이킹 서버가 내부적으로 쓰는 핸들러들을 등록합니다.
			MatchmakingServerWrapper.Install();

			return true;
		}

		public static bool Start()
		{
			Log.Info ("Hello, {0} server is started!", "DediServerManager");
			return true;
		}

		public static bool Uninstall()
		{
			return true;
		}

		public static void OnSessionOpened(Session session)
		{
			Log.Info ("Session opened: session_id={0}", session.Id);
		}

		public static void OnSessionClosed(Session session, Session.CloseReason reason)
		{
			Log.Info ("Session closed: session_id={0}, reason={1}", session.Id, reason);
		}

		public static void SendMyMessage(
				Session session, string message_type, long code, string message, JObject data)
		{
			//
			// 클라이언트에게 보낼 실패 응답을 이 곳에 설정합니다.
			//

			// 로직 상 NULL 세션 값이 오지 않도록 assert 를 추가했습니다.
			Log.Assert(session != null);

			// message 변수를 검사하지 않기 때문에 클라이언트는 빈 message 문자열을 받을 수
			// 있습니다. 만약 빈 문자열을 허용하고 싶지 않다면 아래 assert 를 추가해주세요.
			// Log.Assert(!String.IsNullOrEmpty(message)));

			JObject response = new JObject();
			response["error"] = new JObject();
			response["error"]["code"] = code;
			response["error"]["message"] = message;
			if (data == null)
			{
				// empty object
				response["data"] = new JObject();
			}
			else
			{
				// data 는 반드시 object 형태만 포함합니다. value (문자열, 정수 등) 또는
				// 배열 형태의 JSON 데이터를 허용하지 않게 합니다.
				// LOG_ASSERT(data.IsObject());
				response["data"] = data;
			}

			// 서버가 2개 이상(TCP, HTTP)의 프로토콜을 허용한다면 메시지를 보낼 때
			// 반드시 kTcp 로 프로토콜을 지정해야 합니다. 그렇지 않을 경우
			// 모호한(Ambiguous) 프로토콜로 간주하고 메시지를 보내지 않습니다.
			session.SendMessage(message_type, response, Session.Encryption.kDefault, Session.Transport.kTcp);
		}

		public static void OnLogin(SessionResponse.ResponseResult error, SessionResponse response)
		{
			// 로그인 이후 처리를 담당합니다.
			Log.Info("OnLogin: session={0}, result={1}, data={2}",
							 response.session.Id, (error == SessionResponse.ResponseResult.OK ? "ok" : "failed"),
							 response.data.ToString());

			SendMyMessage(response.session, kLoginMessage, response.error_code,
										response.error_message, response.data);
		}

		public static void OnLogout(
				SessionResponse.ResponseResult error, SessionResponse response, bool caused_by_session_close)
		{
			// 로그아웃 이후 처리를 담당합니다.
			Log.Info("OnLogout: session={0}, result={1}, caused_by={2},, data={3}",
							 response.session.Id, (error == SessionResponse.ResponseResult.OK ? "ok" : "failed"),
							 (caused_by_session_close ? "session close" : "action"),
							 response.data.ToString());

			Event.EventFunction event_fn = new Event.EventFunction(() => {
				MatchmakingHelper.CancelMatchmaking(response.session);

				// 세션 닫힘 이벤트가 아니라면 로그아웃 요청으로 이 콜백을 호출한 경우므로
				// 메시지를 보냅니다.
				if (!caused_by_session_close)
				{
					SendMyMessage(response.session, kLogoutMessage, response.error_code,
												response.error_message, response.data);
				}
			});
			Event.Invoke(event_fn, response.session.Id);
		}

		public static void OnTcpTransportDetached(Session session)
		{
			Log.Info("TCP transport detached: session_id={0}", session.Id);
			// TCP 연결이 닫혔으나 세션은 유효합니다. 이 세션이 매칭 요청을 했다면
			// 매칭을 취소합니다. 클라이언트는 TCP 연결이 끊어졌을 때 매칭을 다시 시도하게
			// 해야 합니다.
			Event.EventFunction event_fn = new Event.EventFunction(() => {
				MatchmakingHelper.CancelMatchmaking(session);
			});
			Event.Invoke(event_fn, session.Id);
		}

		public static void OnLoginRequest(Session session, JObject message)
		{
			Guid session_id = session.Id;
			JObject session_context = session.Context;

			Log.Info("OnLoginRequest: session={0}, context={1}, message={2}",
							 session_id, session_context.ToString(), message.ToString());

			// 로그인을 시도합니다.
			// OnLogin 함수는 이 세션이 로그인을 성공/실패한 결과를 보낼 때 사용합니다.
			// OnLogout 함수는 중복 로그인 처리(로그인한 다른 세션을 로그아웃) 시 사용합니다.
			// 이후 과정은 authentication_helper.cs 를 참고하세요.
			SessionResponse.SessionResponseHandler on_logout =
					new SessionResponse.SessionResponseHandler(
					(SessionResponse.ResponseResult error, SessionResponse response) => {
				OnLogout(error, response, false /* not caused by session close */);
			});
			AuthenticationHelper.Login(session, message, OnLogin, on_logout);
		}

		public static void OnLogoutRequest(Session session, JObject message)
		{
			Guid session_id = session.Id;
			JObject session_context = session.Context;

			Log.Info("OnLogoutRequest: session={0}, context={1}, message={2}",
							 session_id, session_context.ToString(), message.ToString());

			// 이후 과정은 authentication_helper.cs 를 참고하세요.
			SessionResponse.SessionResponseHandler on_logout =
					new SessionResponse.SessionResponseHandler(
					(SessionResponse.ResponseResult error, SessionResponse response) => {
				OnLogout(error, response, false /* not caused by session close */);
			});
			AuthenticationHelper.Logout(session, on_logout);
		}


		public static void OnDedicatedServerSpawned(SessionResponse.ResponseResult error, SessionResponse response)
		{
			// 클라이언트 리다이렉션 메시지는 엔진 내부에서 이 핸들러와 별개로 처리합니다.
			// 이 곳에서는 서버 생성 결과만 메시지를 보냅니다.
			Log.Info(
					"OnDedicatedServerSpawned" +
					": session_id={0}, error={1}, response.error_code={2}, response.error_message={3}, , response.data={4}",
					response.session.Id, (error == SessionResponse.ResponseResult.OK ? "ok" : "failed"),
					response.error_code, response.error_message, response.data.ToString());

			SendMyMessage(response.session, kMatchThenSpawnMessage,
										response.error_code, response.error_message,
										response.data);
		}

		public static void OnMatchThenSpawnRequest(Session session, JObject message)
		{
			Guid session_id = session.Id;
			JObject session_context = session.Context;

			Log.Info("OnMatchThenSpawnRequest: session={0}, context={1}, message={2}",
							 session_id, session_id.ToString(), message.ToString());

			SessionResponse.SessionResponseHandler response_handler =
					new SessionResponse.SessionResponseHandler(
					(SessionResponse.ResponseResult error, SessionResponse response) => {
				SendMyMessage(response.session, kMatchThenSpawnMessage,
											response.error_code, response.error_message,
											response.data);
			});

			// 이후 과정은 matchmaking_helper.cs 를 참고하세요.
			MatchmakingHelper.ProcessSpawnOrMatchmaking(session, message, response_handler);
		}

		public static void OnCancelMatchRequest(Session session, JObject message)
		{
			Guid session_id = session.Id;
			JObject session_context = session.Context;

			Log.Info("OnCancelMatchRequest: session={0}, context={1}, message={2}",
							 session_id, session_context.ToString(), message.ToString());

			SessionResponse.SessionResponseHandler response_handler =
					new SessionResponse.SessionResponseHandler(
					(SessionResponse.ResponseResult error, SessionResponse response) => {
				SendMyMessage(response.session, kCancelMatchMessage,
						response.error_code, response.error_message, response.data);
			});

			// 이후 과정은 matchmaking_helper.cs 를 참고하세요.
			MatchmakingHelper.CancelMatchmaking(session, message, response_handler);
		}
	}
}
