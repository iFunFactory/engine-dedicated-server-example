// Copyright (C) 2020 iFunFactory Inc. All Rights Reserved.
//
// This work is confidential and proprietary to iFunFactory Inc. and
// must not be used, disclosed, copied, or distributed without the prior
// consent of iFunFactory Inc.

using funapi;
using Newtonsoft.Json.Linq;
using System;
using System.Collections.Generic;


namespace DediServerManager
{
	public class AuthenticationHelper
	{
		const string kAccounId = "account_id";
		const string kPlatformName = "platform";
		const string kPlatformAccessToken = "access_token";

		public static void OnLoggedIn(
				string account_id, Session session, bool logged_in, string platform, long try_count,
				SessionResponse.SessionResponseHandler login_handler,
				SessionResponse.SessionResponseHandler logout_handler)
		{
			if (!logged_in) {
				if (try_count == 0)
				{
					// 로그인에 실패했습니다. 누군가 먼저 로그인 하거나, 시스템 장애일 수 있습니다.
					// 처음에는 강제로 로그아웃 후 재시도합니다.
					Log.Info("Already logged in: session_id={0}, account_id={1}, platform={2}, try_count={3}",
									 session.Id, account_id, platform, try_count);

					AccountManager.LogoutCallback on_logged_out =
							new AccountManager.LogoutCallback((string account_id2, Session session_logged_out2, bool logged_out2) => {
						AccountManager.LoginCallback on_logged_in =
								new AccountManager.LoginCallback((string account_id3, Session session3, bool logged_in3) => {
							OnLoggedIn(account_id3, session3, logged_in3, platform, try_count, login_handler, logout_handler);
						});
						AccountManager.CheckAndSetLoggedInAsync(account_id2, session, on_logged_in);

						// 기존 세션에 로그아웃 메시지를 보냅니다.
						if (logged_out2)
						{
							logout_handler(SessionResponse.ResponseResult.OK,
														 new SessionResponse(session_logged_out2, 200, "Duplicated login", new JObject()));
						}
					});

					// 분산 환경이라면 SetLoggedOutGlobalAsync() 함수를 사용해주세요.
					AccountManager.SetLoggedOutAsync(account_id, on_logged_out);
					return;
				}
				else
				{
					// 로그인을 두 번 이상 실패했습니다.
					// 만약 SetLoggedOutGlobalAsync() 함수를 사용 중이라면 분산 환경을
					// 구성하는 Zookeeper / Redis 서비스가 제대로 동작하지 않을 가능성도
					// 있습니다. 그러나 이 경우 엔진 내부적으로 조치한 후 에러를 출력하기
					// 때문에 여기서는 클라이언트 처리만 하는 게 좋습니다.
					Log.Error("Login failed: session_id={0}, account_id={1}, platform={2}, try_count={3}",
										session.Id, account_id, platform, try_count);
					login_handler(SessionResponse.ResponseResult.FAILED,
												new SessionResponse(session, 500, "Internal server error.", new JObject()));
					return;
				}
			}  // if (not logged_in)

			// 로그인 성공
			Log.Info("Login succeed: session_id={0}, account_id={1}, platform={2}, try_count={3}",
							 session.Id, account_id, platform, try_count);

			// 클라이언트에게 보낼 응답은 이 곳에 설정합니다.
			JObject response_data = new JObject();
			response_data["key1"] = "value1";
			response_data["key2"] = "value2";
			response_data["key3"] = "value3";

			login_handler(SessionResponse.ResponseResult.OK, new SessionResponse(session, 200, "OK", response_data));
		}

		public static void OnLoggedOut(
				string account_id, Session session, bool logged_out,
				SessionResponse.SessionResponseHandler logout_handler)
		{
			if (!logged_out)
			{
				// logged_out=false 면 로그아웃하지 않은 사용자가 로그아웃을 시도한 경우입니다.
				Log.Info("Not logged in: session_id={0}, account_id={1}", session.Id, account_id);
				logout_handler(SessionResponse.ResponseResult.FAILED,
											 new SessionResponse(session, 400, "The user did not login.", new JObject()));
				return;
			}

			Log.Info("Logged out: session_id={0}, account_id={1}", session.Id, account_id);

			logout_handler(SessionResponse.ResponseResult.OK, new SessionResponse(session, 200, "OK", new JObject()));
		}

		public static void Login(
				Session session, JObject message,
				SessionResponse.SessionResponseHandler login_handler,
				SessionResponse.SessionResponseHandler logout_handler)
		{
			Log.Assert(session != null);
			Log.Assert(message != null);
			Log.Assert(login_handler != null);
			Log.Assert(logout_handler != null);

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
			if (message[kAccounId] == null || message[kAccounId].Type != JTokenType.String ||
					message[kPlatformName] == null || message[kPlatformName].Type != JTokenType.String)
			{
				Log.Error("The message does not have '{0}' or '{1}': session={2}, message={3}",
									kAccounId, kPlatformName, session.Id, message.ToString());
				login_handler(SessionResponse.ResponseResult.FAILED,
											new SessionResponse(session, 400, "Missing required fields.", new JObject()));
				return;
			}

			string account_id = message[kAccounId].Value<string>();
			string platform = message[kPlatformName].Value<string>();

			if (platform == "facebook")
			{
				// Facebook 플랫폼 사용자의 경우, 올바른 사용자인지 검증합니다.
				if (message[kPlatformAccessToken] == null || message[kPlatformAccessToken].Type != JTokenType.String)
				{
					Log.Error("The message does not have {0}: session={1}, message={2}",
										kPlatformAccessToken, session.Id, message.ToString());
					login_handler(SessionResponse.ResponseResult.FAILED,
												new SessionResponse(session, 400, "Missing required fields.", new JObject()));
					return;
				}

				string access_token = message[kPlatformAccessToken].Value<string>();
				FacebookAuthentication.AuthenticationRequest request =
						new FacebookAuthentication.AuthenticationRequest(access_token);

				FacebookAuthentication.AuthenticationResponseHandler on_authenticated =
						new FacebookAuthentication.AuthenticationResponseHandler(
						(FacebookAuthentication.AuthenticationRequest request2,
						 FacebookAuthentication.AuthenticationResponse response2,
						 bool error) => {
					if (error)
					{
						// Facebook 서버 오류 또는 올바르지 않은 사용자인 경우
						Log.Warning("Failed to authenticate Facebook account: session={0}, code={1}, message={2}",
												session.Id, response2.Error.Code, response2.Error.Message);
						login_handler(SessionResponse.ResponseResult.FAILED,
													new SessionResponse(session, 400, "Missing required fields.", new JObject()));
						return;
					}

					Log.Info("Facebook authentication succeed: session={0}, account_id={1}", session.Id, account_id);

					// 이 예제에서는 로그인 시도를 기록합니다.
					long try_count = 0;

					// 분산 환경이라면 CheckAndSetLoggedInGlobalAsync() 함수를 사용해주세요.
					AccountManager.LoginCallback on_logged_in =
							new AccountManager.LoginCallback((string account_id2, Session session2, bool logged_in2) => {
						OnLoggedIn(account_id2, session2, logged_in2, platform, try_count, login_handler, logout_handler);
					});
					AccountManager.CheckAndSetLoggedInAsync(account_id, session, on_logged_in);
				});

				// Facebook 인증을 요청합니다.
				FacebookAuthentication.Authenticate(request, on_authenticated);
			}
			else
			{
				//
				// 로그인 시도
				//
				// 요청한 세션으로 로그인을 시도합니다. 이 서버의 로그인 정책은 로그인을 시도하되,
				// 이미 다른 곳에서 로그인한 경우, 로그아웃 후 재시도합니다.
				long try_count = 0;

				// 분산 환경이라면 CheckAndSetLoggedInGlobalAsync() 함수를 사용해주세요.
				AccountManager.LoginCallback on_logged_in =
						new AccountManager.LoginCallback((string account_id2, Session session2, bool logged_in2) => {
					OnLoggedIn(account_id2, session2, logged_in2, platform, try_count, login_handler, logout_handler);
				});
				AccountManager.CheckAndSetLoggedInAsync(account_id, session, on_logged_in);
			}
		}

		public static void Logout(Session session, SessionResponse.SessionResponseHandler logout_handler)
		{
			Log.Assert(session != null);
			Log.Assert(logout_handler != null);

			string account_id = AccountManager.FindLocalAccount(session);
			if (String.IsNullOrEmpty(account_id))
			{
				// 이 세션으로 로그인한 적이 없습니다.
				Log.Info("This session was not used for login: session_id={0}", session.Id);
				return;
			}

			// 분산 환경이라면 SetLoggedOutGlobalAsync() 함수를 사용해주세요.
			AccountManager.LogoutCallback logout_cb = new AccountManager.LogoutCallback(
					(string account_id2, Session session2, bool logged_out2) => {
				OnLoggedOut(account_id2, session2, logged_out2, logout_handler);
			});

			AccountManager.SetLoggedOutAsync(account_id, logout_cb);
		}
	}
}
