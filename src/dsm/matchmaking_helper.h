// Copyright (C) 2018-2019 iFunFactory Inc. All Rights Reserved.
//
// This work is confidential and proprietary to iFunFactory Inc. and
// must not be used, disclosed, copied, or distributed without the prior
// consent of iFunFactory Inc.

#ifndef SRC_DSM_MATCHMAKING_HELPER_H_
#define SRC_DSM_MATCHMAKING_HELPER_H_

#include <funapi.h>

#include "session_response.h"


namespace dsm {

class MatchmakingHelper {
 public:
  static void ProcessMatchmaking(
      const Ptr<Session> &session,
      const Json &message,
      const SessionResponseHandler &handler);

  // 'message' 안에 매칭 타입이 있는 경우 사용
  static void CancelMatchmaking(
      const Ptr<Session> &session,
      const Json &message,
      const SessionResponseHandler &handler);

  // 이 세션으로 요청한 매칭이 있으면 취소하는 함수
  // 로그아웃 상태에서도 사용할 수 있습니다.
  static void CancelMatchmaking(const Ptr<Session> &session);
};

}  // namespace dsm


#endif  // SRC_DSM_MATCHMAKING_HELPER_H_
