// Copyright (C) 2018-2019 iFunFactory Inc. All Rights Reserved.
//
// This work is confidential and proprietary to iFunFactory Inc. and
// must not be used, disclosed, copied, or distributed without the prior
// consent of iFunFactory Inc.

#ifndef SRC_BOT_SIMPLE_BOT_CLIENT_H_
#define SRC_BOT_SIMPLE_BOT_CLIENT_H_

#include <funapi.h>

#include <src/dsm/matchmaking_type.h>

namespace bot {

class SimpleBotClient {
 public:
  static void Install(
      int threads, int bot_clients, const dsm::MatchType match_type);
  static void Uninstall();

  static void Start();
};

}  // namespace bot


#endif  // SRC_BOT_SIMPLE_BOT_CLIENT_H_
