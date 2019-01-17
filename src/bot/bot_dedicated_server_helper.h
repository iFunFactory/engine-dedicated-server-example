// Copyright (C) 2018 iFunFactory Inc. All Rights Reserved.
//
// This work is confidential and proprietary to iFunFactory Inc. and
// must not be used, disclosed, copied, or distributed without the prior
// consent of iFunFactory Inc.

#ifndef SRC_BOT_BOT_DEDICATED_SERVER_HELPER_H_
#define SRC_BOT_BOT_DEDICATED_SERVER_HELPER_H_

#include <funapi.h>
#include <funapi/test/network.h>


namespace bot {

class BotDedicatedServerHelper {
 public:
  typedef function<void (
      const Ptr<funtest::Session> &session,
      const string &host,
      const int64_t port,
      const string &token)> RedirectionHandler;

  static void Install(const RedirectionHandler &redirection_handler);
};

}  // namespace bot

#endif  // SRC_BOT_BOT_DEDICATED_SERVER_HELPER_H_
