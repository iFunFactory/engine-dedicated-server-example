// Copyright (C) 2018-2019 iFunFactory Inc. All Rights Reserved.
//
// This work is confidential and proprietary to iFunFactory Inc. and
// must not be used, disclosed, copied, or distributed without the prior
// consent of iFunFactory Inc.

#ifndef SRC_BOT_BOT_MATCHMAKING_HELPER_H_
#define SRC_BOT_BOT_MATCHMAKING_HELPER_H_

#include <funapi.h>
#include <funapi/test/network.h>

#include <src/dsm/matchmaking_type.h>


namespace bot {

class BotMatchmakingHelper {
 public:
  typedef function<void (
      const bool succeed,
      const Ptr<funtest::Session> &session)> MatchmakingHandler;

  static void Install(const MatchmakingHandler &matchmaking_handler);

  static void StartMatchmaking(
      const Ptr<funtest::Session> &session,
      const dsm::MatchType &match_type);
};

}  // namespace bot


#endif  // SRC_BOT_BOT_MATCHMAKING_HELPER_H_
