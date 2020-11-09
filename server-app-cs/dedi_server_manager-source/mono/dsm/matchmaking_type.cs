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
	public class MatchmakingType
	{
		// 유저 레벨
		public const string kMatchLevel = "level";
		// 유저 랭킹 점수
		public const string kMMRScore = "mmr_score";

		public enum MatchType
		{
			kNoMatching = 0,
			kMatch1vs1 = 1,
			kMatch3v3 = 3,
			kMatch6v6 = 6,
		};

		public static bool IsValidMatchType(long match_type_int)
		{
			return match_type_int == (long)MatchType.kNoMatching ||
					match_type_int == (long)MatchType.kMatch1vs1 ||
					match_type_int == (long)MatchType.kMatch3v3 ||
					match_type_int == (long)MatchType.kMatch6v6;
		}

		public static long GetNumberOfMaxPlayers(long match_type_int)
		{
			long total_players_for_match = 0;
			if (match_type_int == (long)MatchType.kMatch1vs1)
			{
				total_players_for_match = 2;
			}
			else if (match_type_int == (long)MatchType.kMatch3v3)
			{
				total_players_for_match = 6;
			}
			else if (match_type_int == (long)MatchType.kMatch6v6)
			{
				total_players_for_match = 12;
			}
			else if (match_type_int == (long)MatchType.kNoMatching)
			{
				// 매치메이킹 없이 참여할 수 있는 연습 게임에서는
				// 최대 12명까지 접속할 수 있습니다.
				total_players_for_match = 12;
			}

			// 위 조건과 다른 매치 타입이 들어올 수 없으므로 이를 검사합니다.
			Log.Assert(total_players_for_match != 0);
			return total_players_for_match;
		}
	}
}