// Copyright (C) 2020 iFunFactory Inc. All Rights Reserved.
//
// This work is confidential and proprietary to iFunFactory Inc. and
// must not be used, disclosed, copied, or distributed without the prior
// consent of iFunFactory Inc.

using funapi;
using funapi.Matchmaking;
using Newtonsoft.Json.Linq;
using System;
using System.Collections.Generic;


namespace DediServerManager
{
	public class MatchmakingServerWrapper
	{
		const string kBlueTeam = "blue_team";

		const string kRedTeam = "red_team";

		static bool CheckPlayerRequirements(funapi.Matchmaking.Player player, funapi.Matchmaking.Match match)
		{
			//
			// 매치 조건 검사 핸들러 함수
			//

			// 플레이어가 이 매치에 참여해도 괜찮은 지 검사합니다.
			// 다음 링크를 참고해 단계 별 매칭, 그룹 매칭 등 여러 조건에 대한 예제를
			// 확인할 수 있습니다.
			// https://ifunfactory.com/engine/documents/reference/ko/contents-support-matchmaking.html#id17

			string account_id = player.Id;

			//
			// 메치메이킹 서버는 StartMatchmaking2 에서 넣은 데이터를 다음 형태로 가공합니다.
			// request_time, elapsed_time 값은 엔진에서 자동으로 추가하고 관리합니다.
			//
			//
			// {
			//   "request_time":6447934119, // 요청 시각
			//   "elapsed_time":0,          // 요청 이후 지난 시간(단위: 초)
			//   "user_data": {
			//     // StartMatchmaking2() 안에 넣은 user_data
			//   }
			//}
			//
			JObject user_context = player.Context;
			JObject user_data = player.Context["user_data"].ToJObject();

			Log.Assert(user_context["request_time"] != null && user_context["request_time"].Type == JTokenType.Integer);
			Log.Assert(user_context["elapsed_time"] != null && user_context["elapsed_time"].Type == JTokenType.Integer);

			Log.Assert(user_data[MatchmakingType.kMatchLevel] != null && user_context["request_time"].Type == JTokenType.Integer,
								 String.Format(": user_data={0}", user_data.ToString()));
			Log.Assert(user_data[MatchmakingType.kMMRScore] != null && user_context["elapsed_time"].Type == JTokenType.Integer,
								 String.Format(": user_data={0}", user_data.ToString()));

			Log.Info("Checking the first condition.");

			// 조건 1. 매치 상대가 없으면 (자신만 있다면) 바로 매치에 참여합니다.
			if (match.Players.Count == 1)
			{
				Log.Info("[Condition 1] A new player is going to join the match: match_id={0}, account_id={1}, user_context={2}",
								 match.MatchId, player.Id, player.Context.ToString());
				return true;
			}

			Log.Info("Checking the second condition.");

			// 조건 2. 매치메이킹에 참여중인 플레이어 중 1명이 30초 이상 기다린 경우 바로 넣습니다.
			for (int i = 0; i < match.Players.Count; ++i)
			{
				if (match.Players[i].Id == account_id)
				{
					// 조건을 검사할 때 나 자신은 제외합니다.
					continue;
				}

				JObject user_context2 = match.Players[i].Context;
				long elapsed_sec = user_context2["elapsed_time"].Value<long>();
				if (elapsed_sec >= 30)
				{
					Log.Info("[Condition 2] A new player is going to join the match: match_id={0}, account_id={1}, user_context={2}",
									 match.MatchId, player.Id, player.Context.ToString());
					return true;
				}
			}

			Log.Info("Checking the third condition.");

			// 조건 3. 평균 레벨 차가 10 이상이거나, 랭킹 점수 차가 100점 이상인 경우
			// 매치에 참여시키지 않습니다.
			long my_match_level = user_data[MatchmakingType.kMatchLevel].Value<long>();
			long my_ranking_score = user_data[MatchmakingType.kMMRScore].Value<long>();

			long avg_match_level = 0, avg_ranking_score = 0;

			for (int i = 0; i < match.Players.Count; ++i)
			{
				if (match.Players[i].Id == account_id)
				{
					// 조건을 검사할 때 나 자신은 제외합니다.
					continue;
				}

				JObject user_context2 = match.Players[i].Context;
				JObject user_data2 = user_context2["user_data"].ToJObject();
				avg_match_level += user_data2[MatchmakingType.kMatchLevel].Value<long>();
				avg_ranking_score += user_data2[MatchmakingType.kMMRScore].Value<long>();
			}

			avg_match_level /= (match.Players.Count);
			avg_ranking_score /= (match.Players.Count);

			Log.Info("my_match_level={0}, my_ranking_score={1}, avg_match_level={2}, avg_ranking_score={3}",
							 my_match_level, my_ranking_score, avg_match_level, avg_ranking_score);

			if (Math.Abs(my_match_level - avg_match_level) >= 10 /* 10점 이상 */ ||
					Math.Abs(my_ranking_score - avg_ranking_score) >= 100 /* 100점 이상*/)
			{
				return false;
			}

			// 매치 조건에 만족하는 플레이어를 찾았습니다.
			Log.Info("[Condition 3] A new player is going to join the match: match_id={0}, account_id={1}, user_context={2}",
							 match.MatchId, player.Id, player.Context.ToString());
			return true;
		}

		static funapi.Matchmaking.Server.MatchState CheckMatchRequirements(funapi.Matchmaking.Match match)
		{
			//
			// 매치 완료 조건 검사 핸들러 함수입니다.
			//

			long total_players_for_match = MatchmakingType.GetNumberOfMaxPlayers(match.MatchType);

			// 총 플레이어 수가 매치 완료 조건에 부합하는 지 검사합니다.
			if (match.Players.Count != total_players_for_match)
			{
				// 아직 더 많은 플레이어가 필요합니다.
				Log.Info("Waiting for more players: match_id={0}, match_type={1}, total_players_for_match={2}, current players={3}",
								 match.MatchId, match.MatchType, total_players_for_match, match.Players.Count);

				return funapi.Matchmaking.Server.MatchState.kMatchNeedMorePlayer;
			}

			Log.Info("Matchmaking is done: match_id={0}, match_type={1}, total_players_for_match={2}, current players={3}",
								match.MatchId, match.MatchType, total_players_for_match, match.Players.Count);

			// 매치메이킹이 끝났으니 이 정보를 토대로 데디케이티드 서버 생성을 요청합니다.
			DedicatedServerHelper.SpawnDedicatedServer(match);
			return funapi.Matchmaking.Server.MatchState.kMatchComplete;
		}

		static void OnPlayerJoined(funapi.Matchmaking.Player player, funapi.Matchmaking.Match match)
		{
			//
			// 플레이어를 매치에 포함한 후 호출하는 핸들러 함수 입니다.
			//
			// CheckPlayerRequirements() 함수 안에서 true 를 반환하면 이 함수를 호출합니다.
			//
			// 이 예제에서는 레드 / 블루로 나눠진 팀에 각각 플레이어를 넣습니다.
			// 블루 팀 인원수가 레드 팀 인원 수보다 많지 않는 한 레드 팀에 우선적으로
			// 플레이어를 넣습니다.

			// 매치를 처음 생성할 때는 매치 컨텍스트가 비어있는 상태이므로 초기화가 필요합니다.
			if (!match.Context.HasValues) {
				match.Context[kBlueTeam] = new JArray();
				match.Context[kRedTeam] = new JArray();
			}

			Log.Info("OnPlayerJoined: player={0}, user_data={1}, match_data={2}",
							 player.Id, player.Context.ToString(), match.Context.ToString());

			if (((JArray)match.Context[kBlueTeam]).Count > ((JArray)match.Context[kRedTeam]).Count)
			{
				((JArray)match.Context[kRedTeam]).Add(player.Id);
			}
			else
			{
				((JArray)match.Context[kBlueTeam]).Add(player.Id);
			}
		}

		static void OnPlayerLeft(funapi.Matchmaking.Player player, funapi.Matchmaking.Match match)
		{
			Log.Assert(match.Context != null);
			Log.Assert(match.Context[kBlueTeam].Type == JTokenType.Array);
			Log.Assert(match.Context[kRedTeam].Type == JTokenType.Array);

			Log.Info("OnPlayerLeft: player={0}, user_data={1}, match_data={2}",
							 player.Id, player.Context.ToString(), match.Context.ToString());

			{
				// 매치메이킹 도중 나간 플레이어가 블루 팀에 있는지 확인합니다.
				JArray array = (JArray)match.Context[kBlueTeam];
				for (int i = 0; i < array.Count; ++i)
				{
					if (array[i].ToString() == player.Id)
					{
						((JArray)match.Context[kBlueTeam]).RemoveAt(i);
						return;
					}
				}
			}

			{
				// 매치메이킹 도중 나간 플레이어가 레드 팀에 있는지 확인합니다.
				JArray array = (JArray)match.Context[kRedTeam];
				for (int i = 0; i < array.Count; ++i)
				{
					if (array[i].ToString() == player.Id)
					{
						((JArray)match.Context[kRedTeam]).RemoveAt(i);
						return;
					}
				}
			}
		}

		public static void Install()
		{
			funapi.Matchmaking.Server.Start(
					CheckPlayerRequirements, CheckMatchRequirements, OnPlayerJoined, OnPlayerLeft);
		}
	}
}