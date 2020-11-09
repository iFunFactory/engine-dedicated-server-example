// Copyright (C) 2020 iFunFactory Inc. All Rights Reserved.
//
// This work is confidential and proprietary to iFunFactory Inc. and
// must !be used, disclosed, copied, or distributed without the prior
// consent of iFunFactory Inc.

using funapi;
using Newtonsoft.Json.Linq;
using System;
using System.Collections.Generic;


namespace DediServerManager
{
	public class MatchmakingHelper
	{

		// 매치메이킹 + 스폰 관련 JSON 키 이름 정의
		const string kAccountId = "account_id";
		const string kUserData = "user_data";
		const string kMatchType = "match_type";
		const string kMatchHistory = "match_history";

		static void LogMatchHistory(Session session, string account_id, long match_type)
		{
			// 추후 이 세션을 사용하는 다른 곳에서 세션 컨텍스트를 수정할 수 있습니다.
			// 따라서 세션 컨텍스트는 세션 ID 를 이벤트 태그로 하는 이벤트 위에서만
			// 수정해야 합니다(세션 열림, 닫힘, 메시지 핸들러는 기본적으로 ID를 태그로 사용합니다)
			Log.Assert(Event.GetCurrentEventTag() == session.Id);
			if (session.Context[kMatchHistory] == null)
			{
				session.Context[kMatchHistory] = new JObject();
			}

			// 로그아웃 (AccountManager 를 쓸 수 없는) 상황에서도 접근할 수 있게
			// 계정 정보를 기록해둡니다.
			session.Context[kMatchHistory][kAccountId] = account_id;
			session.Context[kMatchHistory][kMatchType] = match_type;
		}

		static void ClearMatchHistory(Session session)
		{
			// 추후 이 세션을 사용하는 다른 곳에서 세션 컨텍스트를 수정할 수 있습니다.
			// 따라서 세션 컨텍스트는 세션 ID 를 이벤트 태그로 하는 이벤트 위에서만
			// 수정해야 합니다(세션 열림, 닫힘, 메시지 핸들러는 기본적으로 ID를 태그로 사용합니다)
			Log.Assert(Event.GetCurrentEventTag() == session.Id);
			if (session.Context[kMatchHistory] != null)
			{
				session.Context.Remove(kMatchHistory);
			}
		}

		static void OnMatchCompleted(string account_id,
																 funapi.Matchmaking.Match match,
																 funapi.Matchmaking.MatchResult result,
																 Session session,
																 SessionResponse.SessionResponseHandler handler)
		{
			//
			// 매치 결과를 받는 콜백 핸들러입니다. 매치를 요청한 각 플레이어를 대상으로
			// 핸들러를 호출합니다.
			// 매치를 정상적으로 성사한 경우: match_id 로 직렬화 한 이벤트에서 실행합니다.
			// 매치 성사에 실패한 경우: 직렬화하지 않은 이벤트에서 실행합니다.
			//
			// 매치메이킹에 참여하는 사람이 많지 않거나 matchmaking_server_wrapper.cs 에서
			// 정의한 매치 조건이 까다로운 경우, 클라이언트가 매치메이킹을 요청하는 시점과
			// 매치가 성사되는 이 시점의 차가 커질 수 있습니다.
			//
			// 따라서 클라이언트는 매치메이킹 요청을 보낸 후 이 핸들러에서 메시지를 보내기
			// 전까지 다른 행동을 할 수 있도록 만들어야 합니다.
			//

			Log.Info("OnMatchCompleted: account_id={0}, result={1}", account_id, result);

			if (result != funapi.Matchmaking.MatchResult.kSuccess)
			{
				// funapi.Matchmaking.Client.MatchResult 결과에 따라 어떻게 처리할 지 결정합니다.
				if (result == funapi.Matchmaking.MatchResult.kError)
				{
					// 엔진 내부 에러입니다.
					// 일반적으로 RPC 서비스를 사용할 수 없을 경우 발생합니다.
					handler(SessionResponse.ResponseResult.FAILED,
									new SessionResponse(session, 500, "Internal server error.",
									new JObject()));
				}
				else if (result == funapi.Matchmaking.MatchResult.kAlreadyRequested)
				{
					// 이미 이 account_id 로 매치메이킹 요청을 했습니다. 매치 타입이 다르더라도
					// ID 가 같다면 실패합니다.
					handler(SessionResponse.ResponseResult.FAILED,
									new SessionResponse(session, 400, "Already requested.", new JObject()));
				}
				else if (result == funapi.Matchmaking.MatchResult.kTimeout)
				{
					// 매치메이킹 요청을 지정한 시간 안에 수행하지 못했습니다.
					// 더 넓은 범위의 매치 재시도 또는 클라이언트에게 단순 재시도를 요구할 수 있습니다.
					handler(SessionResponse.ResponseResult.FAILED,
									new SessionResponse(session, 400, "Timed out.", new JObject()));
				}
				else
				{
					// 아이펀 엔진에서 추후 MatchResult 를 더 추가할 경우를 대비해
					// 이곳에 로그를 기록합니다.
					Log.Warning("not supported error: result={0}", result);
					handler(SessionResponse.ResponseResult.FAILED,
									new SessionResponse(session, 500, "Unknown error.", new JObject()));
				}
				return;
			}

			// 매치 결과 ID 입니다.
			Guid match_id = match.MatchId;
			// 매치메이킹 서버에서 정의한 JSON 컨텍스트 입니다.
			// 매치메이킹 요청(StartMatchmaking2) 시 인자로 넣는 context 와는 다른
			// context 라는 점에 주의하세요.
			JObject match_context = match.Context;
			// 매치메이킹 요청 시 지정한 매치 타입입니다.
			long match_type = match.MatchType;
			// 이 매치에 참여한 플레이어 정보입니다.
			string player_ss = "[";
			for (int i = 0; i < match.Players.Count; ++i)
			{
				if (i != 0)
				{
					player_ss += ", ";
				}
				player_ss += String.Format("account_id={0}, user_data={1}",
																	 match.Players[i].Id,
																	 match.Players[i].Context.ToString());
			}
			player_ss += "]";

			Log.Info("Matchmaking succeed: match_id={0}, match_context={1}, match_type={2}, players={3}",
							 match_id, match_context.ToString(), match_type, player_ss);

			// 매치메이킹에 성공했습니다. 매치메이킹 서버(matchmaking_server_wrapper.cc)에서
			// 매치에 성공한 사람들을 모아 데디케이티드 서버 생성을 시작합니다.
			// (CheckMatchRequirements 함수에서 kMatchComplete 를 반환한 후입니다)
			// 이 시점에서는 단순히 히스토리만 초기화하고 메시지는 보내지 않습니다.
			// (데디케이티드 서버 생성 후 최종적으로 성공 메시지를 보냅니다)

			Event.EventFunction event_fn = new Event.EventFunction(() => {
				ClearMatchHistory(session);
			});
			Event.Invoke(event_fn, session.Id);
		}

		static void OnMatchProgressUpdated(string account_id,
																			 funapi.Matchmaking.Match match,
																			 string player_id_joined,
																			 string player_id_left)
		{
			Log.Info("OnMatchProgressUpdated: account_id={0}, match_context={1}, " +
							 "player_id_joined={2}, player_id_left={3}",
							 account_id, match.Context.ToString(), player_id_joined, player_id_left);

			// 누군가 매치에 참여하거나 나갔을 때 이 핸들러를 호출합니다.
			// 또는 다른 곳에서 funapi.Matchmaking.Client.UpdateMatchPlayerContext() 를
			// 호출할 때도 이 핸들러를 호출합니다.
			//
			// 참고: UpdateMatchPlayerContext() 함수는 매치를 요청했던 사용자가
			// 자신이 보낸 user_data 내용을 변경할 때 사용할 수 있습니다.

			string ss = "---------------------------------\n";
			for (int i = 0; i <match.Players.Count; ++i)
			{
				if (i != 0)
				{
					ss += ", ";
				}
				ss += String.Format("account_id={0}, user_data={1}",
														match.Players[i].Id,
														match.Players[i].Context.ToString());
			}
			ss += "\n---------------------------------";
			Log.Info(ss);
		}

		static void SpawnOrSendUser(string account_id,
																JObject user_data,
																long match_type)
		{
			Log.Assert(match_type == (long)MatchmakingType.MatchType.kNoMatching);

			// 1. 사람이 없고 인원이 부족한 서버를 찾아 난입을 시도합니다.
			// 2. 난입에 실패한 경우 새 서버를 생성합니다.

			DedicatedServerHelper.SendUserCallback cb =
					new DedicatedServerHelper.SendUserCallback((bool succeed)=> {
				// 난입에 실패했습니다. 새 서버를 생성합니다.
				// 재시도를 하거나, 실패처리할 수 있습니다.
				if (!succeed)
				{
					Log.Info("Spawning a new dedicated server for: account_id={0}", account_id);
					DedicatedServerHelper.SpawnDedicatedServer(account_id, user_data);
					return;
				}

				// 매치 참여에 성공했습니다. 데디케이티드 서버 리다이렉션 메시지는 엔진 내부의
				// 서버 오케스트레이터에서 자동으로 보내기 때문에 처리할 필요가 없습니다.
				// 게임 로직에 따라서는 아래 메시지 전송이 불필요할 수 있습니다.
				// 이 예제에서는 단순히 클라이언트에게 매치 참여 완료 메시지만 전달합니다.
			});

			Log.Info("Trying to send user: account_id={0}", account_id);
			DedicatedServerHelper.SendUser(match_type, account_id, user_data, cb);
		}

		public static void ProcessSpawnOrMatchmaking(
				Session session,
				JObject message,
				SessionResponse.SessionResponseHandler handler)
		{
			Log.Assert(session != null);
			// 메시지 핸들러 함수는 세션 ID 를 이벤트 태그로 하는 이벤트 위에서 실행합니다.
			// 아래는 이 함수를 메시지 핸들러 위에서 실행하게 강제하게 검사하는 식입니다.
			// 세션 ID 별로 이 함수를 직렬화하지 않으면 동시에 서로 다른 곳에서
			// 이 세션에 접근할 수 있습니다.
			Log.Assert(Event.GetCurrentEventTag() == session.Id);

			//
			// 매치메이킹 + 데디케이티드 서버 스폰 요청 예제
			//
			// 유저 2명, 4명, 6명을 대상으로 새로운 데디케이티드 서버 프로세스를 생성합니다.
			//
			// 클라이언트는 다음 메시지 형태로 매치메이킹을 요청합니다.
			// {
			//   // Facebook ID 또는 구글+ ID 등 고유한 ID 를 사용해야 합니다.
			//   "account_id": "id",
			//   // 매치 타입, 이 파일에 있는 MatchType 정의 참조
			//   "match_type": 1
			//   "user_data": {
			//      "level": 70,
			//      "ranking_score": 1500,
			//      ...
			//   },
			// }

			if (message[kAccountId] == null || message[kAccountId].Type != JTokenType.String ||
					message[kMatchType] == null || message[kMatchType].Type != JTokenType.Integer ||
					message[kUserData] == null || message[kUserData].Type != JTokenType.Object)
			{
				Log.Error("Missing required fields: '{0}' / '{1}' / '{2}': session_id={3}, message={4}",
									kAccountId, kMatchType, kUserData, session.Id, message.ToString());
				handler(SessionResponse.ResponseResult.FAILED,
								new SessionResponse(session, 400, "Missing required fields.",
								new JObject()));
				return;
			}

			//
			// 매치메이킹 요청 인자 설정 가이드
			//

			// 1. 매치 타입
			// 매치 타입을 식별하는 용도로 사용합니다. 정해진 값이 없으므로
			// 서버에서 정의한 매치 타입과 일치하는지 확인할 필요가 있습니다.
			long match_type = message[kMatchType].Value<long>();
			if (!MatchmakingType.IsValidMatchType(match_type))
			{
				Log.Error("Invalid match_type: session_id={0}, message={1}", session.Id, message.ToString());
				handler(SessionResponse.ResponseResult.FAILED,
								new SessionResponse(session, 400, "Invalid arguments.", new JObject()));
				return;
			}

			// 2. 계정(account) ID
			// AccountManager 로 로그인한 계정 ID 를 입력합니다.
			string account_id = message[kAccountId].Value<string>();

			// 요청한 계정과 로그인 중인 세션이 일치하는 지 검사합니다.
			// 이 검사를 통해 다른 유저 ID 로 매칭 요청하는 것을 방지할 수 있습니다.
			if (AccountManager.FindLocalSession(account_id).Id != session.Id)
			{
				Log.Info("ProcessMatchmaking denied: bad request: session_id={0}, message={1}",
								 session.Id, message.ToString());
				handler(SessionResponse.ResponseResult.FAILED,
								new SessionResponse(session, 400, "Access denied for this account.", new JObject()));
				return;
			}

			// 3. user_data, 매치메이킹 및 생성한 데디케이티드 서버 안에서 사용할
			// 플레이어의 데이터 입니다. 서버는 클라이언트에서 보낸 user_data 를 복사한 후
			// 요청 시간을 추가합니다. 따라서 매치메이킹 시 사용할 데이터는 최종적으로
			// 다음과 같습니다.
			//   "user_data": {
			//      "level": 70,
			//      "mmr_score": 1500,
			//      "req_time": 1544167339,
			//      ...
			//   },
			JObject user_data = message[kUserData].ToJObject();
			if (user_data[MatchmakingType.kMatchLevel] == null ||
					user_data[MatchmakingType.kMatchLevel].Type != JTokenType.Integer ||
					user_data[MatchmakingType.kMMRScore] == null ||
					user_data[MatchmakingType.kMMRScore].Type != JTokenType.Integer)
			{
				// 매치메이킹 요청에 필요한 인자가 부족합니다.
				Log.Error("Missing required fields: session_id={0}, message={1}", session.Id, message.ToString());
				handler(SessionResponse.ResponseResult.FAILED,
								new SessionResponse(session, 400, "Missing required fields.", new JObject()));
				return;
			}

			// 매치메이킹 없이 스폰을 진행하는 타입이면, 즉시 스폰을 시작합니다.
			// 매치메이킹은 2명 이상인 경우에만 동작한다는 점에 주의해주세요.
			// 1명이 매치메이킹 큐에 들어가면 매치메이킹 콜백을 호출하지 않습니다.
			// 연습 게임 또는 데디케이티드 서버 내부에서 사람을 채운 후 게임을 시작할 때
			// 이러한 방식을 사용할 수 있습니다.
			if (match_type == (long)MatchmakingType.MatchType.kNoMatching)
			{
				SpawnOrSendUser(account_id, user_data, match_type);
				return;
			}

			// 4. OnMatchCompleted 콜백
			// 매칭을 종료했을 때 결과를 받을 콜백 함수를 지정합니다.
			// 매치 결과는 콜백 함수의 funapi.Matchmaking.Client.MatchResult 타입으로
			// 확인할 수 있습니다.

			// 5. 서버 선택 방법
			//    - kRandom: 서버를 랜덤하게 선택합니다.
			//    - kMostNumberOfPlayers; 사람이 가장 많은 서버에서 매치를 수행합니다.
			//    - kLeastNumberOfPlayers; 사람이 가장 적은 서버에서 매치를 수행합니다.
			funapi.Matchmaking.Client.TargetServerSelection target_selection =
					funapi.Matchmaking.Client.TargetServerSelection.kRandom;

			// 6. OnMatchProgressUpdated 콜백
			// 매치 상태를 업데이트 할 때마다 결과를 받을 콜백 함수를 지정합니다.
			// 타임 아웃 (기본 값: default(TimeSpan))
			TimeSpan timeout = default(TimeSpan);

			// 이 세션에서 요청한 매치 타입을 기록해둡니다.
			LogMatchHistory(session, account_id, match_type);

			Log.Info("Requesting a matchmaking: session_id={0}, account_id={1}, match_type={2}, user_data={3}",
							 session.Id, account_id, match_type, user_data.ToString());

			funapi.Matchmaking.Client.MatchCallback match_cb = new funapi.Matchmaking.Client.MatchCallback(
					(string player_id, funapi.Matchmaking.Match match, funapi.Matchmaking.MatchResult result) => {
				OnMatchCompleted(player_id, match, result, session, handler);
			});
			funapi.Matchmaking.Client.Start2(
					match_type, account_id, user_data,
					match_cb,
					target_selection,
					OnMatchProgressUpdated,
					timeout);
		}

		public static void CancelMatchmaking(
				Session session, JObject message, SessionResponse.SessionResponseHandler handler)
		{
			// 메시지 핸들러 함수는 세션 ID 를 이벤트 태그로 하는 이벤트 위에서 실행합니다.
			// 아래는 이 함수를 메시지 핸들러 위에서 실행하게 강제하게 검사하는 식입니다.
			// 세션 ID 별로 이 함수를 직렬화하지 않으면 동시에 서로 다른 곳에서
			// 이 세션에 접근할 수 있습니다.
			Log.Assert(Event.GetCurrentEventTag() == session.Id);

			// 클라이언트는 다음 메시지 형태로 매치메이킹 취소를 요청합니다.
			// {
			//   "account_id": "id",
			//   "match_type": 1
			// }

			if (message[kAccountId] == null || message[kAccountId].Type != JTokenType.String ||
					message[kMatchType] == null || message[kMatchType].Type != JTokenType.Integer)
			{
				Log.Error("Missing required fields: '{0}' / '{1}': session_id={2}, message={3}",
									kAccountId, kMatchType, session.Id, message.ToString());
				handler(SessionResponse.ResponseResult.FAILED,
								new SessionResponse(session, 400, "Missing required fields.", new JObject()));
				return;
			}

			// 매치 타입
			long match_type = message[kMatchType].Value<long>();
			if (!MatchmakingType.IsValidMatchType(match_type))
			{
				Log.Error("Invalid match_type: session_id={0}, message={1}", session.Id, message.ToString());
				handler(SessionResponse.ResponseResult.FAILED,
								new SessionResponse(session, 400, "Invalid arguments.", new JObject()));
				return;
			}

			if (match_type == (long)MatchmakingType.MatchType.kNoMatching)
			{
				// 매치메이킹 기능을 쓰지 않으므로 취소 처리한다.
				handler(SessionResponse.ResponseResult.OK,
								new SessionResponse(session, 200, "OK.", new JObject()));
			}

			// 계정
			string account_id = message[kAccountId].Value<string>();

			// 요청한 계정과 로그인 중인 세션이 일치하는 지 검사합니다.
			// 이 검사를 통해 다른 유저 ID 로 매칭을 취소하는 행위를 방지할 수 있습니다.
			if (AccountManager.FindLocalSession(account_id).Id != session.Id)
			{
				Log.Info("CancelMatchmaking denied: bad request: session_id={0}, message={1}",
								 session.Id, message.ToString());
				handler(SessionResponse.ResponseResult.FAILED,
								new SessionResponse(session, 400, "Access denied for this account.", new JObject()));
				return;
			}

			funapi.Matchmaking.Client.CancelCallback cancel_callback =
					new funapi.Matchmaking.Client.CancelCallback(
					(string account_id2, funapi.Matchmaking.CancelResult result2) => {
				if (result2 != funapi.Matchmaking.CancelResult.kSuccess)
				{
					// kCRNoRequest (요청하지 않은 매치) 또는
					// kCRError (엔진 내부 에러)가 올 수 있습니다.
					handler(SessionResponse.ResponseResult.FAILED,
									new SessionResponse(session, 500, "Internal server error.",
																	new JObject()));
				}
				else
				{
					handler(SessionResponse.ResponseResult.OK,
									new SessionResponse(session, 200, "OK.", new JObject()));
				}
			});
			Log.Info("Canceling matchmaking: session_id={0}, account_id={1}, match_type={2}",
							 session.Id, account_id, match_type);

			funapi.Matchmaking.Client.Cancel(match_type, account_id, cancel_callback);
		}

		public static void CancelMatchmaking(Session session)
		{
			// 메시지 핸들러 함수는 세션 ID 를 이벤트 태그로 하는 이벤트 위에서 실행합니다.
			// 아래는 이 함수를 메시지 핸들러 위에서 실행하게 강제하게 검사하는 식입니다.
			// 세션 ID 별로 이 함수를 직렬화하지 않으면 동시에 서로 다른 곳에서
			// 이 세션에 접근할 수 있습니다.
			Log.Assert(Event.GetCurrentEventTag() == session.Id);

			JObject context = session.Context;
			if (context[kMatchHistory] == null || context[kMatchHistory].Type != JTokenType.Object)
			{
				// 매칭을 요청한 적이 없습니다. 종료합니다.
				return;
			}

			Log.Assert(context[kMatchHistory][kAccountId] != null && context[kMatchHistory][kAccountId].Type == JTokenType.String);
			Log.Assert(context[kMatchHistory][kMatchType] != null && context[kMatchHistory][kMatchType].Type == JTokenType.Integer);

			string account_id = context[kMatchHistory][kAccountId].Value<string>();
			long match_type = context[kMatchHistory][kMatchType].Value<long>();

			funapi.Matchmaking.Client.CancelCallback cancel_callback =
					new funapi.Matchmaking.Client.CancelCallback(
					(string account_id2, funapi.Matchmaking.CancelResult result) => {
				// 아무것도 하지 않습니다.
			});

			Log.Info("Canceling matchmaking(with session context): session_id={0}, account_id={1}, match_type={2}",
							 session.Id, account_id, match_type);

			funapi.Matchmaking.Client.Cancel(match_type, account_id, cancel_callback);
		}
	}
}