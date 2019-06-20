// Copyright (C) 2013-2017 iFunFactory Inc. All Rights Reserved.
//
// This work is confidential and proprietary to iFunFactory Inc. and
// must not be used, disclosed, copied, or distributed without the prior
// consent of iFunFactory Inc.

#include "FunapiPrivatePCH.h"

#ifdef FUNAPI_UE4_PLATFORM_PS4

#include "funapi_http.h"
#include "funapi_plugin.h"
#include "funapi_utils.h"
#include "funapi_tasks.h"

#include <sceerror.h>
#include <scetypes.h>
#include <net.h>
#include <libhttp.h>

#define SSL_HEAP_SIZE	(1024 * 1024)
#define HTTP_HEAP_SIZE	(256 * 1024)
#define NET_HEAP_SIZE	(16 * 1024)
#define USER_AGENT	"Funapi/1"

namespace fun {

////////////////////////////////////////////////////////////////////////////////
// FunapiHttpImpl implementation.

class FunapiHttpImpl : public std::enable_shared_from_this<FunapiHttpImpl> {
 public:
  typedef std::function<void(void*, const size_t)> ResponseCallback;
  typedef FunapiHttp::CompletionHandler CompletionHandler;
  typedef FunapiHttp::ErrorHandler ErrorHandler;
  typedef FunapiHttp::ProgressHandler ProgressHandler;
  typedef FunapiHttp::DownloadCompletionHandler DownloadCompletionHandler;
  typedef FunapiHttp::HeaderFields HeaderFields;

  FunapiHttpImpl();
  FunapiHttpImpl(const fun::string &cert);
  virtual ~FunapiHttpImpl();

  void PostRequest(const fun::string &url,
                   const HeaderFields &header,
                   const fun::vector<uint8_t> &body,
                   const ErrorHandler &error_handler,
                   const CompletionHandler &completion_handler);

  void SetTimeout(const long seconds);

 private:
  void Init();
  void Cleanup();

  int libnet_memid_ = -1;
  int libssl_ctxid_ = -1;
  int libhttp_ctxid_ = -1;

  long connect_timeout_seconds_ = 5;

  fun::string ca_cert_;
};


FunapiHttpImpl::FunapiHttpImpl() : ca_cert_("") {
  Init();
}


FunapiHttpImpl::FunapiHttpImpl(const fun::string &cert) : ca_cert_(cert) {
  Init();
}


FunapiHttpImpl::~FunapiHttpImpl() {
  Cleanup();
  // DebugUtils::Log("%s", __FUNCTION__);
}


void FunapiHttpImpl::Init() {
  int ret = -1;

  static int index = 0;
  fun::stringstream ss;
  ss << "FunapiHttp-" << ++index;

  ret = sceNetPoolCreate(ss.str().c_str(), NET_HEAP_SIZE, 0);
  if (ret < 0) {
    DebugUtils::Log("[FunapiHttp] %s,%d ret=%x\n", __FUNCTION__, __LINE__, ret);
    return;
  }
  libnet_memid_ = ret;

  ret = sceSslInit(SSL_HEAP_SIZE);
  if (ret < 0) {
    DebugUtils::Log("sceSslInit() error: 0x%08X\n", ret);
    return;
  }
  libssl_ctxid_ = ret;

  ret = sceHttpInit(libnet_memid_, libssl_ctxid_, HTTP_HEAP_SIZE);
  if (ret < 0) {
    DebugUtils::Log("sceHttpInit() error: 0x%08X\n", ret);
    return;
  }
  libhttp_ctxid_ = ret;

  if (!ca_cert_.empty()) {
    SceSslData* caList;
    SceSslData caCert;
    caCert.ptr = (char*)ca_cert_.data();
    caCert.size = ca_cert_.size();
    caList = &caCert;

    ret = sceHttpsLoadCert(libhttp_ctxid_, 1, (const SceSslData**)&caList, NULL, NULL);
    if (ret < 0) {
      DebugUtils::Log("sceHttpsLoadCert() error: 0x%08X\n", ret);
      return;
    }
  }
}


void FunapiHttpImpl::Cleanup() {
  SceInt32 ret;

  if (libnet_memid_ > -1) {
    ret = sceNetPoolDestroy(libnet_memid_);
    if (ret < 0) {
      DebugUtils::Log("%s,%d ret=%x\n", __FUNCTION__, __LINE__, ret);
    }
  }

  if (libhttp_ctxid_ > -1) {
    ret = sceHttpTerm(libhttp_ctxid_);
    if (ret < 0) {
      DebugUtils::Log("sceHttpEnd() error: 0x%08X\n", ret);
    }
  }

  if (libssl_ctxid_ > -1) {
    ret = sceSslTerm(libssl_ctxid_);
    if (ret < 0) {
      DebugUtils::Log("sceSslEnd() error: 0x%08X\n", ret);
    }
  }
}


void FunapiHttpImpl::PostRequest(const fun::string &url,
                                 const HeaderFields &header,
                                 const fun::vector<uint8_t> &body,
                                 const ErrorHandler &error_handler,
                                 const CompletionHandler &completion_handler) {
  fun::vector<fun::string> header_receiving;
  fun::vector<uint8_t> body_receiving;

  int tmplId = 0, connId = 0, reqId = 0, statusCode = 0;
  uint64_t contentLength;

  auto delete_function = [&reqId, &connId, &tmplId]() {
    int ret;

    if (reqId > 0) {
      ret = sceHttpDeleteRequest(reqId);
      if (ret < 0) {
        DebugUtils::Log("sceHttpDeleteRequest() error: 0x%08X\n", ret);
      }
    }
    if (connId > 0) {
      ret = sceHttpDeleteConnection(connId);
      if (ret < 0) {
        DebugUtils::Log("sceHttpDeleteConnection() error: 0x%08X\n", ret);
      }
    }
    if (tmplId > 0) {
      ret = sceHttpDeleteTemplate(tmplId);
      if (ret < 0) {
        DebugUtils::Log("sceHttpDeleteTemplate() error: 0x%08X\n", ret);
      }
    }
  };

  int ret;

  ret = sceHttpCreateTemplate(libhttp_ctxid_, USER_AGENT, SCE_HTTP_VERSION_1_1, SCE_TRUE);
  if (ret < 0) {
    delete_function();
    error_handler(ret, "sceHttpCreateTemplate() error");
    return;
  }
  tmplId = ret;

  if (ca_cert_.empty()) {
    ret = sceHttpsDisableOption(tmplId, SCE_HTTPS_FLAG_SERVER_VERIFY);
    if (ret < 0) {
      delete_function();
      error_handler(ret, "sceHttpsDisableOption() error");
      return;
    }
  }

  ret = sceHttpCreateConnectionWithURL(tmplId, url.c_str(), SCE_TRUE);
  if (ret < 0) {
    delete_function();
    error_handler(ret, "sceHttpCreateConnectionWithURL() error");
    return;
  }
  connId = ret;

  ret = sceHttpSetConnectTimeOut(connId, 1000000 * connect_timeout_seconds_);
  if (ret < 0) {
    delete_function();
    error_handler(ret, "sceHttpSetConnectTimeOut() error");
    return;
  }

  ret = sceHttpCreateRequestWithURL(connId, SCE_HTTP_METHOD_POST, url.c_str(), body.size());
  if (ret < 0) {
    delete_function();
    error_handler(ret, "sceHttpCreateConnectionWithURL() error");
    return;
  }
  reqId = ret;

  for (auto it : header) {
    ret = sceHttpAddRequestHeader(reqId, it.first.c_str(), it.second.c_str(), SCE_HTTP_HEADER_OVERWRITE);
    if (ret < 0) {
      delete_function();
      error_handler(ret, "sceHttpAddRequestHeader() error");
      return;
    }
  }

  ret = sceHttpSendRequest(reqId, body.data(), body.size());
  if (ret < 0) {
    delete_function();
    char temp_buffer[1024];
    sprintf(temp_buffer, "sceHttpSendRequest() error (errno=%d)", ret);
    error_handler(ret, temp_buffer);
    return;
  }

  ret = sceHttpGetStatusCode(reqId, &statusCode);
  if (ret < 0) {
    delete_function();
    error_handler(ret, "sceHttpGetStatusCode() error");
    return;
  }
  if (statusCode != 200) {
    delete_function();
    fun::stringstream ss;
    ss << "http response code " << statusCode;
    error_handler(statusCode, ss.str());
    return;
  }

  char *headers;
  size_t headerSize = 0;
  ret = sceHttpGetAllResponseHeaders(reqId, &headers, &headerSize);
  if (ret < 0) {
    delete_function();
    error_handler(ret, "sceHttpGetResponseContentLength() error");
    return;
  }

  int start_index = 0;
  int end_index = 0;
  for (int i = 0; i < headerSize; ++i) {
    if (headers[i] == '\r') {
      if (headers[i + 1] == '\n') {
        end_index = i + 1;
        header_receiving.push_back(fun::string(headers + start_index, headers + end_index));
        start_index = end_index + 1;
        i = end_index;
      }
    }
  }

  int contentLengthType;
  ret = sceHttpGetResponseContentLength(reqId, &contentLengthType, &contentLength);
  if (ret < 0) {
    delete_function();
    error_handler(ret, "sceHttpGetResponseContentLength() error");
    return;
  }

  body_receiving.resize(contentLength);

  ret = sceHttpReadData(reqId, body_receiving.data(), contentLength);
  if (ret < 0) {
    delete_function();
    error_handler(ret, "sceHttpReadData() error");
    return;
  }

  delete_function();

  completion_handler(header_receiving, body_receiving);
}


void FunapiHttpImpl::SetTimeout(const long seconds)
{
  connect_timeout_seconds_ = seconds;
}

////////////////////////////////////////////////////////////////////////////////
// FunapiHttp implementation.


FunapiHttp::FunapiHttp() : impl_(std::make_shared<FunapiHttpImpl>()) {
}


FunapiHttp::FunapiHttp(const fun::string &path) : impl_(std::make_shared<FunapiHttpImpl>(path)) {
}


FunapiHttp::~FunapiHttp() {
  // DebugUtils::Log("%s", __FUNCTION__);
}


std::shared_ptr<FunapiHttp> FunapiHttp::Create() {
  return std::make_shared<FunapiHttp>();
}


std::shared_ptr<FunapiHttp> FunapiHttp::Create(const fun::string &path) {
  return std::make_shared<FunapiHttp>(path);
}


void FunapiHttp::PostRequest(const fun::string &url,
                             const HeaderFields &header,
                             const fun::vector<uint8_t> &body,
                             const ErrorHandler &error_handler,
                             const CompletionHandler &completion_handler) {
  impl_->PostRequest(url, header, body, error_handler, completion_handler);
}


void FunapiHttp::GetRequest(const fun::string &url,
                const HeaderFields &header,
                const ErrorHandler &error_handler,
                            const CompletionHandler &completion_handler) {
}


void FunapiHttp::DownloadRequest(const fun::string &url,
                                 const fun::string &path,
                                 const HeaderFields &header,
                                 const ErrorHandler &error_handler,
                                 const ProgressHandler &progress_handler,
                                 const DownloadCompletionHandler &download_completion_handler) {
}


void FunapiHttp::SetTimeout(const long seconds)
{
  impl_->SetTimeout(seconds);
}

}  // namespace fun

#endif