// Copyright (C) 2018 iFunFactory Inc. All Rights Reserved.
//
// This work is confidential and proprietary to iFunFactory Inc. and
// must not be used, disclosed, copied, or distributed without the prior
// consent of iFunFactory Inc.

#ifndef DEDI_SERVER_EXAMPLE_ENGINE_SRC_BOT_BOT_CLIENT_H_
#define DEDI_SERVER_EXAMPLE_ENGINE_SRC_BOT_BOT_CLIENT_H_

#include <funapi.h>


namespace bot {

class SimpleBotClient {
 public:
  static void Install(int threads, int bot_clients);
  static void Uninstall();

  static void Start();
};

}  // namespace bot


#endif  // DEDI_SERVER_EXAMPLE_ENGINE_SRC_BOT_BOT_CLIENT_H_
