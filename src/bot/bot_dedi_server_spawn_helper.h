// Copyright (C) 2018 iFunFactory Inc. All Rights Reserved.
//
// This work is confidential and proprietary to iFunFactory Inc. and
// must not be used, disclosed, copied, or distributed without the prior
// consent of iFunFactory Inc.

#ifndef DEDI_SERVER_EXAMPLE_ENGINE_SRC_BOT_DEDI_SPAWN_HELPER_H_
#define DEDI_SERVER_EXAMPLE_ENGINE_SRC_BOT_DEDI_SPAWN_HELPER_H_

#include <funapi.h>
#include <funapi/test/network.h>


namespace bot {

class BotDediServerSpawnHelper {
 public:
  typedef function<void (
      const bool succeed,
      const Ptr<funtest::Session> &session)> SpawnHandler;

  typedef function<void (
      const Ptr<funtest::Session> &session,
      const string &host,
      const int64_t port,
      const string &token)> RedirectionHandler;

  static void Install(const SpawnHandler &spawn_handler,
                      const RedirectionHandler &redirection_handler);

  static void Spawn(const Ptr<funtest::Session> &session);
};

}  // namespace bot

#endif  // DEDI_SERVER_EXAMPLE_ENGINE_SRC_BOT_DEDI_SPAWN_HELPER_H_
