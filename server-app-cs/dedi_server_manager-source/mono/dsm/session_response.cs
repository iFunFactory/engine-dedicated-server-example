// Copyright (C) 2020 iFunFactory Inc. All Rights Reserved.
//
// This work is confidential and proprietary to iFunFactory Inc. and
// must not be used, disclosed, copied, or distributed without the prior
// consent of iFunFactory Inc.

using funapi;
using Newtonsoft.Json.Linq;
using System;
using System.Collections.Generic;


namespace DediServerManager
{
	public class SessionResponse
	{
			public enum ResponseResult
			{
				OK = 0,
				FAILED = 1
			}

			public SessionResponse(Session session_in, long error_code_in, string error_message_in, JObject data_in)
			{
				session = session_in;
				error_code = error_code_in;
				error_message = error_message_in;
				data = data_in;
			}

			public delegate void SessionResponseHandler(ResponseResult session, SessionResponse message);

			public Session session;
			public long error_code;
			public string error_message;
			public JObject data;
	}
}