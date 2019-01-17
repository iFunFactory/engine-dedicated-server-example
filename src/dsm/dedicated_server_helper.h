// Copyright (C) 2018-2019 iFunFactory Inc. All Rights Reserved.
//
// This work is confidential and proprietary to iFunFactory Inc. and
// must not be used, disclosed, copied, or distributed without the prior
// consent of iFunFactory Inc.

#ifndef SRC_DSM_DEDICATED_SERVER_HELPER_H_
#define SRC_DSM_DEDICATED_SERVER_HELPER_H_

#include <funapi.h>

#include "session_response.h"

namespace dsm {

class DedicatedServerHelper {
 public:
  static void Install(
      const SessionResponseHandler &response_handler);

  // 1인 데디케이티드 서버 생성
  static void SpawnDedicatedServer(
      const string &account_id,
      const Json &user_data);

  static void SpawnDedicatedServer(
      const MatchmakingServer::Match &match);

};

}  // namespace dsm

#endif  // SRC_DSM_DEDICATED_SERVER_HELPER_H_
