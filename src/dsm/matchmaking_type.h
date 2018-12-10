// Copyright (C) 2018 iFunFactory Inc. All Rights Reserved.
//
// This work is confidential and proprietary to iFunFactory Inc. and
// must not be used, disclosed, copied, or distributed without the prior
// consent of iFunFactory Inc.


#ifndef DEDI_SERVER_EXAMPLE_ENGINE_SRC_DSM_MATCHMAKING_SERVER_TYPE_H_
#define DEDI_SERVER_EXAMPLE_ENGINE_SRC_DSM_MATCHMAKING_SERVER_TYPE_H_

#include <funapi.h>


namespace dsm {

enum MatchType {
  kMatch1vs1 = 1,
  kMatch3v3 = 3,
  kMatch6v6 = 6
};


inline bool IsValidMatchType(const int64_t match_type_int) {
  return match_type_int == kMatch1vs1 ||
      match_type_int == kMatch3v3 ||
      match_type_int == kMatch6v6;
}

// 매치메이킹 서버, 클라이언트에서 사용할 값 목록

// 유저 레벨
extern const char *kMatchLevel;

// 유저 랭킹 점수
extern const char *kRankingScore;
}

#endif // DEDI_SERVER_EXAMPLE_ENGINE_SRC_DSM_MATCHMAKING_SERVER_TYPE_H_
