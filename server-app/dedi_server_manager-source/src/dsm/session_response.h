// Copyright (C) 2018-2019 iFunFactory Inc. All Rights Reserved.
//
// This work is confidential and proprietary to iFunFactory Inc. and
// must not be used, disclosed, copied, or distributed without the prior
// consent of iFunFactory Inc.

#ifndef SRC_DSM_SESSION_RESPONSE_H_
#define SRC_DSM_SESSION_RESPONSE_H_

#include <funapi.h>

namespace dsm {

enum class ResponseResult : int {
  OK = 0,
  FAILED = 1
};

struct SessionResponse {
  SessionResponse(const Ptr<Session> &session_in,
                  const int64_t error_code_in,
                  const string &error_message_in,
                  const Json &data_in) :
      session(session_in),
      error_code(error_code_in),
      error_message(error_message_in),
      data(data_in) {
  }

  const Ptr<Session> session;
  const int64_t error_code;
  const string error_message;
  const Json data;
};

typedef function<void (
    const ResponseResult error,
    const SessionResponse &response)> SessionResponseHandler;

}  // namespace dsm

#endif  // SRC_DSM_SESSION_RESPONSE_H_
