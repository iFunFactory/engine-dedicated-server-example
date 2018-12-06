// Copyright (C) 2018 iFunFactory Inc. All Rights Reserved.
//
// This work is confidential and proprietary to iFunFactory Inc. and
// must not be used, disclosed, copied, or distributed without the prior
// consent of iFunFactory Inc.

#ifndef DEDI_SERVER_EXAMPLE_ENGINE_SRC_DSM_MATCHMAKING_SERVER_WRAPPER_H_
#define DEDI_SERVER_EXAMPLE_ENGINE_SRC_DSM_MATCHMAKING_SERVER_WRAPPER_H_

#include <funapi.h>

#include <src/dsm/session_response.h>


namespace dsm {

class MatchmakingServerWrapper {
 public:
  static void Start();
};

}  // namespace dsm


#endif  // DEDI_SERVER_EXAMPLE_ENGINE_SRC_DSM_MATCHMAKING_SERVER_WRAPPER_H_