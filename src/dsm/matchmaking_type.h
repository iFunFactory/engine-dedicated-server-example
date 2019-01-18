// Copyright (C) 2018-2019 iFunFactory Inc. All Rights Reserved.
//
// This work is confidential and proprietary to iFunFactory Inc. and
// must not be used, disclosed, copied, or distributed without the prior
// consent of iFunFactory Inc.


#ifndef SRC_DSM_MATCHMAKING_TYPE_H_
#define SRC_DSM_MATCHMAKING_TYPE_H_

#include <funapi.h>


namespace dsm {

enum MatchType {
  kNoMatching = 0,
  kMatch1vs1 = 1,
  kMatch3v3 = 3,
  kMatch6v6 = 6,
};


inline bool IsValidMatchType(const int64_t match_type_int) {
  return match_type_int == kNoMatching ||
      match_type_int == kMatch1vs1 ||
      match_type_int == kMatch3v3 ||
      match_type_int == kMatch6v6;
}


inline size_t GetNumberOfMaxPlayers(const int64_t match_type_int) {
  size_t total_players_for_match = 0;
  if (match_type_int == kMatch1vs1) {
    total_players_for_match = 2;
  } else if (match_type_int == kMatch3v3) {
    total_players_for_match = 6;
  } else if (match_type_int == kMatch6v6) {
    total_players_for_match = 12;
  } else if (match_type_int == kNoMatching) {
    // 매치메이킹 없이 참여할 수 있는 연습 게임에서는
    // 최대 12명까지 접속할 수 있습니다.
    total_players_for_match = 12;
  }

  // 위 조건과 다른 매치 타입이 들어올 수 없으므로 이를 검사합니다.
  LOG_ASSERT(total_players_for_match != 0);
  return total_players_for_match;
}

// 매치메이킹 서버, 클라이언트에서 사용할 값 목록

// 유저 레벨
extern const char *kMatchLevel;

// 유저 랭킹 점수
extern const char *kMMRScore;

}

#endif  // SRC_DSM_MATCHMAKING_TYPE_H_
