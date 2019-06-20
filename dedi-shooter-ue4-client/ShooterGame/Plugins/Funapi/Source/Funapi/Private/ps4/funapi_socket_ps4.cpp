// Copyright (C) 2013-2017 iFunFactory Inc. All Rights Reserved.
//
// This work is confidential and proprietary to iFunFactory Inc. and
// must not be used, disclosed, copied, or distributed without the prior
// consent of iFunFactory Inc.

#include "FunapiPrivatePCH.h"

#ifdef FUNAPI_UE4_PLATFORM_PS4

#include "funapi_socket.h"
#include "funapi_plugin.h"
#include "funapi_utils.h"

#define UI UI_ST
THIRD_PARTY_INCLUDES_START
#include "openssl/ssl.h"
#include "openssl/err.h"
THIRD_PARTY_INCLUDES_END
#undef UI

// PS4
#include <net.h>
#include <libssl.h>

namespace fun {

////////////////////////////////////////////////////////////////////////////////
// FunapiAddrInfoImpl implementation.

class FunapiAddrInfoImpl : public std::enable_shared_from_this<FunapiAddrInfoImpl> {
 public:
   FunapiAddrInfoImpl();
  virtual ~FunapiAddrInfoImpl();

  fun::string GetString();

  SceNetSockaddrIn GetAddrInfo();
  void SetAddrInfo(const SceNetSockaddrIn &info);

 private:
  SceNetSockaddrIn addrinfo_res_;
};


FunapiAddrInfoImpl::FunapiAddrInfoImpl() {
}


FunapiAddrInfoImpl::~FunapiAddrInfoImpl() {
  // DebugUtils::Log("%s", __FUNCTION__);
}


void FunapiAddrInfoImpl::SetAddrInfo(const SceNetSockaddrIn &info) {
  addrinfo_res_ = info;
}


SceNetSockaddrIn FunapiAddrInfoImpl::GetAddrInfo() {
  return addrinfo_res_;
}


fun::string FunapiAddrInfoImpl::GetString() {
  auto sin = addrinfo_res_;

  char addrStr[SCE_NET_INET_ADDRSTRLEN];

  if (sceNetInetNtop(SCE_NET_AF_INET,
    &sin.sin_addr, addrStr, sizeof(addrStr)) == NULL) {
    return "NULL";
  }

  return fun::string(addrStr);
}


////////////////////////////////////////////////////////////////////////////////
// FunapiSocketImpl implementation.

class FunapiSocketImpl : public std::enable_shared_from_this<FunapiSocketImpl> {
 public:
  FunapiSocketImpl();
  virtual ~FunapiSocketImpl();

  static fun::string GetStringFromAddrInfo(const SceNetSockaddrIn &sin);

  static fun::vector<std::weak_ptr<FunapiSocketImpl>> vec_sockets_;
  static std::mutex vec_sockets_mutex_;

  static void Add(std::shared_ptr<FunapiSocketImpl> s);
  static fun::vector<std::shared_ptr<FunapiSocketImpl>> GetSocketImpls();
  static bool Select();

  static const int kBufferSize = 65536;

  int GetSocket();
  virtual bool IsReadySelect();
  virtual void OnSelect(const fd_set rset,
                        const fd_set wset,
                        const fd_set eset) = 0;

 protected:
  bool InitAddrInfo(int socktype,
                    const char* hostname_or_ip,
                    const int port,
                    int &error_code,
                    fun::string &error_string);
  void FreeAddrInfo();

  bool InitSocket(int socktype,
                  int &error_code,
                  fun::string &error_string);
  void CloseSocket();

  void SocketSelect(const fd_set rset,
                    const fd_set wset,
                    const fd_set eset);

  virtual void OnSend() = 0;
  virtual void OnRecv() = 0;

  SceNetId socket_ = -1;
  SceNetSockaddrIn addrinfo_;
  SceNetSockaddrIn addrinfo_res_;
};


fun::string FunapiSocketImpl::GetStringFromAddrInfo(const SceNetSockaddrIn &sin) {
  char addrStr[SCE_NET_INET_ADDRSTRLEN];

  if (sceNetInetNtop(SCE_NET_AF_INET,
    &sin.sin_addr, addrStr, sizeof(addrStr)) == NULL) {
    return "NULL";
  }

  return fun::string(addrStr);
}


fun::vector<std::weak_ptr<FunapiSocketImpl>> FunapiSocketImpl::vec_sockets_;
std::mutex FunapiSocketImpl::vec_sockets_mutex_;


void FunapiSocketImpl::Add(std::shared_ptr<FunapiSocketImpl> s) {
  std::unique_lock<std::mutex> lock(vec_sockets_mutex_);
  vec_sockets_.push_back(s);
}


fun::vector<std::shared_ptr<FunapiSocketImpl>> FunapiSocketImpl::GetSocketImpls() {
  fun::vector<std::shared_ptr<FunapiSocketImpl>> v_sockets;
  fun::vector<std::weak_ptr<FunapiSocketImpl>> v_weak_sockets;
  {
    std::unique_lock<std::mutex> lock(vec_sockets_mutex_);
    if (!vec_sockets_.empty()) {
      for (auto i : vec_sockets_) {
        if (auto s = i.lock()) {
          v_sockets.push_back(s);
          v_weak_sockets.push_back(i);
        }
      }

      vec_sockets_.swap(v_weak_sockets);
    }
  }

  return v_sockets;
}


bool FunapiSocketImpl::Select() {
  auto v_sockets = FunapiSocketImpl::GetSocketImpls();

  if (!v_sockets.empty()) {
    int max_fd = -1;

    fd_set rset;
    fd_set wset;
    fd_set eset;

    FD_ZERO(&rset);
    FD_ZERO(&wset);
    FD_ZERO(&eset);

    fun::vector<std::shared_ptr<FunapiSocketImpl>> v_select_sockets;
    for (auto s : v_sockets)
    {
      if (s->IsReadySelect())
      {
        int fd = s->GetSocket();
        if (fd > 0) {
          if (fd > max_fd) max_fd = fd;

          FD_SET(fd, &rset);
          FD_SET(fd, &wset);
          FD_SET(fd, &eset);

          v_select_sockets.push_back(s);
        }
      }
    }

    if (!v_select_sockets.empty())
    {
      struct timeval timeout = { 0, 0 };
      if (select(max_fd + 1, &rset, &wset, &eset, &timeout) > 0)
      {
        for (auto s : v_select_sockets) {
          s->OnSelect(rset, wset, eset);
        }
      }

      return true;
    }
  }

  return false;
}


FunapiSocketImpl::FunapiSocketImpl() {
}


FunapiSocketImpl::~FunapiSocketImpl() {
  CloseSocket();
  FreeAddrInfo();
  // DebugUtils::Log("%s", __FUNCTION__);
}


int FunapiSocketImpl::GetSocket() {
  return socket_;
}


bool FunapiSocketImpl::IsReadySelect() {
  return true;
}


void FunapiSocketImpl::FreeAddrInfo() {
}


bool FunapiSocketImpl::InitAddrInfo(int socktype,
                                    const char* hostname_or_ip,
                                    const int port,
                                    int &error_code,
                                    fun::string &error_string) {
  SceNetSockaddrIn sin;
  memset(&sin, 0, sizeof(sin));
  sin.sin_len = sizeof(sin);
  sin.sin_family = SCE_NET_AF_INET;
  if (sceNetInetPton(SCE_NET_AF_INET, hostname_or_ip, &sin.sin_addr) == 0) {
    bool is_failed = false;
    char temp_buffer[1024];
    temp_buffer[0] = 0;

    SceNetInAddr *addr = &sin.sin_addr;
    SceNetId rid = -1;
    int memid = -1;
    int ret;

    if (false == is_failed) {
      ret = sceNetPoolCreate(__FUNCTION__, 4 * 1024, 0);
      if (ret < 0) {
        sprintf(temp_buffer, "sceNetPoolCreate() failed (0x%x errno=%d)\n",
          ret, sce_net_errno);
        is_failed = true;
      }
    }

    if (false == is_failed) {
      memid = ret;
      ret = sceNetResolverCreate("resolver", memid, 0);
      if (ret < 0) {
        sprintf(temp_buffer, "sceNetResolverCreate() failed (0x%x errno=%d)\n",
          ret, sce_net_errno);
        is_failed = true;
      }
    }

    if (false == is_failed) {
      rid = ret;
      ret = sceNetResolverStartNtoa(rid, hostname_or_ip, addr, 0, 0, 0);
      if (ret < 0) {
        sprintf(temp_buffer, "sceNetResolverStartNtoa() failed (0x%x errno=%d)\n",
          ret, sce_net_errno);
        is_failed = true;
      }
    }

    if (false == is_failed) {
      ret = sceNetResolverDestroy(rid);
      if (ret < 0) {
        sprintf(temp_buffer, "sceNetResolverDestroy() failed (0x%x errno=%d)\n",
          ret, sce_net_errno);
        is_failed = true;
      }
    }

    if (false == is_failed) {
      ret = sceNetPoolDestroy(memid);
      if (ret < 0) {
        sprintf(temp_buffer, "sceNetPoolDestroy() failed (0x%x errno=%d)\n",
          ret, sce_net_errno);
        is_failed = true;
      }
    }

    if (is_failed) {
      sceNetResolverDestroy(rid);
      sceNetPoolDestroy(memid);

      error_code = sce_net_errno;
      error_string = temp_buffer;

      return false;
    }
  }

  sin.sin_port = sceNetHtons(port);

  addrinfo_ = sin;

  return true;
}


void FunapiSocketImpl::CloseSocket() {
  if (socket_ >= 0) {
    // DebugUtils::Log("Socket [%d] closed.", socket_);

    sceNetSocketClose(socket_);

    socket_ = -1;
  }
}


bool FunapiSocketImpl::InitSocket(int socktype,
                                  int &error_code,
                                  fun::string &error_string) {
  auto s = sceNetSocket(__FUNCTION__, SCE_NET_AF_INET, socktype, 0);
  if (s < 0) {
    char temp_buffer[1024];
    sprintf(temp_buffer, "sceNetSocket() failed (errno=%d)\n", sce_net_errno);

    error_string = temp_buffer;
    error_code = sce_net_errno;

    return false;
  }

  socket_ = s;

  return true;
}


void FunapiSocketImpl::SocketSelect(const fd_set rset,
                                    const fd_set wset,
                                    const fd_set eset) {
  if (socket_ > 0) {
    if (FD_ISSET(socket_, &rset)) {
      OnRecv();
    }
  }

  if (socket_ > 0) {
    if (FD_ISSET(socket_, &wset)) {
      OnSend();
    }
  }
}


////////////////////////////////////////////////////////////////////////////////
// FunapiTcpImpl implementation.

class FunapiTcpImpl : public FunapiSocketImpl {
 public:
  typedef FunapiTcp::ConnectCompletionHandler ConnectCompletionHandler;
  typedef FunapiTcp::RecvHandler RecvHandler;
  typedef FunapiTcp::SendHandler SendHandler;
  typedef FunapiTcp::SendCompletionHandler SendCompletionHandler;

  FunapiTcpImpl();
  virtual ~FunapiTcpImpl();

  void OnSelect(const fd_set rset, const fd_set wset, const fd_set eset);

  void Connect(const char* hostname_or_ip,
               const int port,
               const time_t connect_timeout_seconds,
               const bool disable_nagle,
               const ConnectCompletionHandler &connect_completion_handler,
               const SendHandler &send_handler,
               const RecvHandler &recv_handler);

  void Connect(const char* hostname_or_ip,
               const int sever_port,
               const time_t connect_timeout_seconds,
               const bool disable_nagle,
               const bool use_tls,
               const fun::string &cert,
               const ConnectCompletionHandler &connect_completion_handler,
               const SendHandler &send_handler,
               const RecvHandler &recv_handler);

  void Connect(SceNetSockaddrIn addrinfo_res,
               ConnectCompletionHandler connect_completion_handler);

  void Connect(SceNetSockaddrIn addrinfo_res);

  bool Send(const fun::vector<uint8_t> &body, SendCompletionHandler send_handler);

  bool IsReadySelect();

 protected:
  bool InitTcpSocketOption(bool disable_nagle,
                           int &error_code,
                           fun::string &error_string);

  void SocketSelect(const fd_set rset,
                    const fd_set wset,
                    const fd_set eset);

  void OnConnectCompletion(const bool is_failed,
                           const bool is_timed_out,
                           const int error_code,
                           const fun::string &error_string);

  bool ConnectTLS();
  void CleanupSSL();

  void OnSend();
  void OnRecv();

  enum class SocketSelectState : int {
    kNone = 0,
    kSelect,
  };
  SocketSelectState socket_select_state_ = SocketSelectState::kNone;

  fun::vector<std::function<void(const fd_set rset,
                                 const fd_set wset,
                                 const fd_set eset)>> on_socket_select_;

  ConnectCompletionHandler completion_handler_ = nullptr;

  SendHandler send_handler_;
  RecvHandler recv_handler_;
  SendCompletionHandler send_completion_handler_;

  fun::vector<uint8_t> body_;
  int offset_ = 0;
  time_t connect_timeout_seconds_ = 5;

  fun::string ca_cert_;
  bool use_tls_ = false;

  SSL_CTX *ctx_ = nullptr;
  SSL *ssl_ = nullptr;
};


FunapiTcpImpl::FunapiTcpImpl() {
}


FunapiTcpImpl::~FunapiTcpImpl() {
  // DebugUtils::Log("%s", __FUNCTION__);
  CleanupSSL();
}


void FunapiTcpImpl::CleanupSSL() {
  // DebugUtils::Log("%s", __FUNCTION__);

  if (ssl_) {
    SSL_shutdown(ssl_);
    SSL_free(ssl_);
    ssl_ = nullptr;
  }

  if (ctx_) {
    SSL_CTX_free(ctx_);
    ctx_ = nullptr;
  }
}


void FunapiTcpImpl::OnSelect(const fd_set rset,
                             const fd_set wset,
                             const fd_set eset) {
  if (socket_select_state_ == SocketSelectState::kSelect) {
    FunapiSocketImpl::SocketSelect(rset, wset, eset);
  }
}


bool FunapiTcpImpl::IsReadySelect() {
  if (socket_select_state_ == SocketSelectState::kSelect) {
    return true;
  }

  return false;
}


void FunapiTcpImpl::Connect(SceNetSockaddrIn addrinfo_res) {
  addrinfo_res_ = addrinfo_res;

  socket_select_state_ = SocketSelectState::kNone;

  int rc = sceNetConnect(socket_, (SceNetSockaddr *)&addrinfo_res_,sizeof(addrinfo_res_));
  if (rc < 0 && sce_net_errno != SCE_NET_EINPROGRESS) {
    OnConnectCompletion(true, false, sce_net_errno, "");
    return;
  }

  fd_set rset;
  fd_set wset;
  fd_set eset;

  FD_ZERO(&rset);
  FD_ZERO(&wset);
  FD_ZERO(&eset);

  FD_SET(socket_, &rset);
  FD_SET(socket_, &wset);
  FD_SET(socket_, &eset);

  struct timeval timeout = { connect_timeout_seconds_, 0 };
  rc = select(socket_+1, &rset, &wset, &eset, &timeout);
  if (rc < 0) {
    // select failed
    OnConnectCompletion(true, false, sce_net_errno, "");
  }
  else if (rc == 0) {
    // connect timed out
    OnConnectCompletion(true, true, sce_net_errno, "");
  }
  else {
    int err;
    SceNetSocklen_t optlen = sizeof(err);

    if (!FD_ISSET(socket_, &rset) && !FD_ISSET(socket_, &wset)) {
      OnConnectCompletion(true, false, 0, "");
      return;
    }

    // obtains a pending error value
    // SCE_NET_SO_ERROR optval datatype is int
    if (sceNetGetsockopt(socket_, SCE_NET_SOL_SOCKET, SCE_NET_SO_ERROR, &err, &optlen) < 0) {
      OnConnectCompletion(true, false, err, "sceNetGetsockopt() failed");
      return;
    }

    // Normal case
    if (err == 0) {
      OnConnectCompletion(false, false, 0, "");
      return;
    }

    OnConnectCompletion(true, false, err, "sceNetConnect() failed");
  }
}


void FunapiTcpImpl::Connect(SceNetSockaddrIn addrinfo_res,
                            ConnectCompletionHandler connect_completion_handler) {
  completion_handler_ = connect_completion_handler;

  Connect(addrinfo_res);
}


void FunapiTcpImpl::Connect(const char* hostname_or_ip,
                            const int port,
                            const time_t connect_timeout_seconds,
                            const bool disable_nagle,
                            const ConnectCompletionHandler &connect_completion_handler,
                            const SendHandler &send_handler,
                            const RecvHandler &recv_handler) {
  Connect(hostname_or_ip,
          port,
          connect_timeout_seconds,
          disable_nagle,
          false, "",
          connect_completion_handler,
          send_handler,
          recv_handler);
}


void FunapiTcpImpl::Connect(const char* hostname_or_ip,
                            const int port,
                            const time_t connect_timeout_seconds,
                            const bool disable_nagle,
                            const bool use_tls,
                            const fun::string &cert,
                            const ConnectCompletionHandler &connect_completion_handler,
                            const SendHandler &send_handler,
                            const RecvHandler &recv_handler) {
  completion_handler_ = connect_completion_handler;

  if (socket_ != -1) {
    OnConnectCompletion(true, false, 0, "");
    return;
  }

  use_tls_ = use_tls;
  ca_cert_ = cert;

  if (use_tls_) {
    SSL_load_error_strings();
  }

  send_handler_ = send_handler;
  recv_handler_ = recv_handler;
  connect_timeout_seconds_ = connect_timeout_seconds;

  int error_code = 0;
  fun::string error_string;

  if (!InitAddrInfo(SCE_NET_SOCK_STREAM, hostname_or_ip, port, error_code, error_string)) {
    OnConnectCompletion(true, false, error_code, error_string);
    return;
  }

  addrinfo_res_ = addrinfo_;

  if (!InitSocket(SCE_NET_SOCK_STREAM, error_code, error_string)) {
    OnConnectCompletion(true, false, error_code, error_string);
    return;
  }

  if (!InitTcpSocketOption(disable_nagle, error_code, error_string)) {
    OnConnectCompletion(true, false, error_code, error_string);
    return;
  }

  // log
  {
    fun::string hostname = FunapiSocketImpl::GetStringFromAddrInfo(addrinfo_res_);
    DebugUtils::Log("Address Info: %s -> %s", hostname_or_ip, hostname.c_str());
  }
  // //

  Connect(addrinfo_res_);
}


bool FunapiTcpImpl::InitTcpSocketOption(bool disable_nagle, int &error_code, fun::string &error_string) {
  char temp_buffer[1024];
  int optval = 1;
  int ret = sceNetSetsockopt(socket_, SCE_NET_SOL_SOCKET, SCE_NET_SO_NBIO, &optval, sizeof(optval));

  if (ret < 0) {
    sprintf(temp_buffer, "sceNetSetsockopt(SO_NBIO) failed (errno=%d)\n", sce_net_errno);

    error_code = sce_net_errno;
    error_string = temp_buffer;

    return false;
  }

  if (disable_nagle) {
    ret = sceNetSetsockopt(socket_, SCE_NET_IPPROTO_TCP, SCE_NET_TCP_NODELAY, &optval, sizeof(optval));

    if (ret < 0) {
      sprintf(temp_buffer, "sceNetSetsockopt(TCP_NODELAY) failed (errno=%d)\n", sce_net_errno);

      error_code = sce_net_errno;
      error_string = temp_buffer;

      return false;
    }
  }

  return true;
}


bool FunapiTcpImpl::ConnectTLS() {
  auto on_ssl_error_completion = [this]() {
    unsigned long error_code = ERR_get_error();

    char error_buffer[1024];
    ERR_error_string(error_code, (char *)error_buffer);

    OnConnectCompletion(true, false, static_cast<int>(error_code), error_buffer);
  };

  auto add_cert = [](SSL_CTX *ctx, const void* data, int len) -> bool {
    BIO *bio;
    X509 *cert = nullptr;

    if ((bio = BIO_new(BIO_s_mem())) == NULL) {
      return false;
    }

    BIO_write(bio, data, len);
    cert = PEM_read_bio_X509_AUX(bio, NULL, NULL, NULL);
    BIO_free(bio);

    if (cert == NULL) {
      return false;
    }

    X509_STORE *store;
    store = SSL_CTX_get_cert_store((SSL_CTX *)ctx);
    int ret = X509_STORE_add_cert(store, cert);
    X509_free(cert);

    if (ret == 0) {
      return false;
    }

    return true;
  };

  static auto ssl_init = FunapiInit::Create([](){
    SSL_library_init();
  });

  CleanupSSL();

  SSL_METHOD *method = (SSL_METHOD *)SSLv23_client_method();

  if (!method) {
    on_ssl_error_completion();
    return false;
  }

  ctx_ = SSL_CTX_new(method);

  if (!ctx_) {
    on_ssl_error_completion();
    return false;
  }

  int ret = 0;

  // user root ca
  if (!ca_cert_.empty()) {
    if (false == add_cert(ctx_, ca_cert_.data(), ca_cert_.size())) {
      on_ssl_error_completion();
      return false;
    }
  }

  // ps4 root ca
  {
    bool is_error = false;
    char error_buffer[1024];

    const int32 PS4_SSL_HEAP_SIZE = 256 * 1024;
    int libssl_ctxid = sceSslInit(PS4_SSL_HEAP_SIZE);
    if (libssl_ctxid < 0) {
      sprintf(error_buffer, "sceSslInit() error: 0x%08X\n", libssl_ctxid);
      is_error = true;
    }

    SceSslCaCerts caCerts;

    if (false == is_error) {
      memset(&caCerts, 0, sizeof(SceSslCaCerts));
      ret = sceSslGetCaCerts(libssl_ctxid, &caCerts);
      if (ret < 0) {
        sprintf(error_buffer, "sceSslGetCaCerts() error: 0x%08X\n", ret);
        is_error = true;
      }
    }

    if (false == is_error) {
      for (int i = 0; i < caCerts.certDataNum; ++i) {
        SceSslData &temp_data = caCerts.certData[i];
        if (false == add_cert(ctx_, temp_data.ptr, temp_data.size)) {
          sprintf(error_buffer, "X509_STORE_add_cert() error");
          is_error = true;
        }
      }
    }

    if (libssl_ctxid > -1) {
      ret = sceSslTerm(libssl_ctxid);
      if (ret < 0) {
        sprintf(error_buffer, "sceSslTerm() error: 0x%08X\n", ret);
        is_error = true;
      }
    }

    if (is_error) {
      OnConnectCompletion(true, false, static_cast<int>(0), error_buffer);
      return false;
    }
  }

  ssl_ = SSL_new(ctx_);

  if (!ssl_) {
    on_ssl_error_completion();
    return false;
  }

  ret = SSL_set_fd(ssl_, socket_);
  if (ret != 1) {
    on_ssl_error_completion();
    return false;
  }

  while (true)
  {
    ret = SSL_connect(ssl_);
    if (ret != 1) {
      int n = SSL_get_error(ssl_, ret);

      if (n == SSL_ERROR_WANT_READ || n == SSL_ERROR_WANT_READ) {
        continue;
      }
      else {
        on_ssl_error_completion();
        return false;
      }
    }
    else {
      break;
    }
  }

  // verify server certificate
  {
    bool is_error = false;

    X509* cert = SSL_get_peer_certificate(ssl_);

    if (cert != NULL)
    {
      X509_free(cert);

      ret = SSL_get_verify_result(ssl_);
      if (ret != X509_V_OK) {
        is_error = true;
      }
    }
    else {
      is_error = true;
    }

    if (is_error) {
      auto verify_error = SSL_get_verify_result(ssl_);
      OnConnectCompletion(true, false, static_cast<int>(verify_error), X509_verify_cert_error_string(verify_error));
      return false;
    }
  }

  return true;
}


void FunapiTcpImpl::OnConnectCompletion(const bool is_failed,
                                        const bool is_timed_out,
                                        const int error_code,
                                        const fun::string &error_string) {
  if (false == is_failed && use_tls_) {
    if (false == ConnectTLS()) {
      return;
    }
  }

  if (completion_handler_) {
    if (!is_failed) {
      socket_select_state_ = SocketSelectState::kSelect;
    }
    else {
      socket_select_state_ = SocketSelectState::kNone;
    }

    auto addrinfo = FunapiAddrInfo::Create();
    addrinfo->GetImpl()->SetAddrInfo(addrinfo_res_);

    completion_handler_(is_failed, is_timed_out, error_code, error_string, addrinfo);
  }
}


void FunapiTcpImpl::OnSend() {
  if (body_.empty() && offset_ == 0) {
    send_handler_();
  }

  if (!body_.empty()) {
    int nSent = 0;

    if (use_tls_) {
      nSent = static_cast<int>(SSL_write(ssl_,
        reinterpret_cast<char*>(body_.data()) + offset_,
        body_.size() - offset_));
    }
    else {
      nSent = static_cast<int>(sceNetSend(socket_,
        reinterpret_cast<char*>(body_.data()) + offset_,
        body_.size() - offset_,
        0));
    }

    /*
    if (nSent == 0) {
      DebugUtils::Log("Socket [%d] closed.", socket_);
    }
    */

    if (nSent <= 0) {
      send_completion_handler_(true, sce_net_errno, "sceNetSend() failed", nSent);
      CloseSocket();
    }
    else {
      offset_ += nSent;
    }

    if (offset_ == body_.size()) {
      offset_ = 0;
      body_.resize(0);

      send_completion_handler_(false, 0, "", nSent);
    }
  }
}


void FunapiTcpImpl::OnRecv() {
  fun::vector<uint8_t> buffer(kBufferSize);

  int nRead = 0;

  if (use_tls_) {
    nRead = static_cast<int>(SSL_read(ssl_, reinterpret_cast<char*>(buffer.data()), kBufferSize));
  }
  else {
    nRead = static_cast<int>(sceNetRecv(socket_, reinterpret_cast<char*>(buffer.data()), kBufferSize, 0));
  }

  /*
  if (nRead == 0) {
    DebugUtils::Log("Socket [%d] closed.", socket_);
  }
  */

  if (nRead <= 0) {
    recv_handler_(true, sce_net_errno, "sceNetRecv() failed", nRead, buffer);
    CloseSocket();
  }
  else {
    recv_handler_(false, 0, "", nRead, buffer);
  }
}


bool FunapiTcpImpl::Send(const fun::vector<uint8_t> &body, SendCompletionHandler send_completion_handler) {
  send_completion_handler_ = send_completion_handler;

  body_.insert(body_.end(), body.cbegin(), body.cend());

//  // log
//  {
//    fun::string temp_string(body.cbegin(), body.cend());
//    printf("\"%s\"\n", temp_string.c_str());
//  }
//  //

  return true;
}


////////////////////////////////////////////////////////////////////////////////
// FunapiUdpImpl implementation.

class FunapiUdpImpl : public FunapiSocketImpl {
 public:
  typedef FunapiUdp::InitHandler InitHandler;
  typedef FunapiUdp::RecvHandler RecvHandler;
  typedef FunapiUdp::SendHandler SendHandler;
  typedef FunapiUdp::SendCompletionHandler SendCompletionHandler;

  FunapiUdpImpl() = delete;
  FunapiUdpImpl(const char* hostname_or_ip,
                const int port,
                const InitHandler &init_handler,
                const SendHandler &send_handler,
                const RecvHandler &recv_handler);
  virtual ~FunapiUdpImpl();

  void OnSelect(const fd_set rset, const fd_set wset, const fd_set eset);
  bool Send(const fun::vector<uint8_t> &body, const SendCompletionHandler &send_handler);

 private:
  void Finalize();
  void OnSend();
  void OnRecv();

  SendHandler send_handler_;
  RecvHandler recv_handler_;
};


FunapiUdpImpl::FunapiUdpImpl(const char* hostname_or_ip,
                             const int port,
                             const InitHandler &init_handler,
                             const SendHandler &send_handler,
                             const RecvHandler &recv_handler)
: send_handler_(send_handler), recv_handler_(recv_handler) {
  int error_code = 0;
  fun::string error_string;

  if (!InitAddrInfo(SCE_NET_SOCK_DGRAM, hostname_or_ip, port, error_code, error_string)) {
    init_handler(true, error_code, error_string);
    return;
  }

  addrinfo_res_ = addrinfo_;

  if (!InitSocket(SCE_NET_SOCK_DGRAM, error_code, error_string)) {
    init_handler(true, error_code, error_string);
    return;
  }

  init_handler(false, 0, "");
}


FunapiUdpImpl::~FunapiUdpImpl() {
}


void FunapiUdpImpl::OnSelect(const fd_set rset, const fd_set wset, const fd_set eset) {
  FunapiSocketImpl::SocketSelect(rset, wset, eset);
}


void FunapiUdpImpl::OnSend() {
  send_handler_();
}


void FunapiUdpImpl::OnRecv() {
  fun::vector<uint8_t> receiving_vector(kBufferSize);

  size_t len = sizeof(addrinfo_res_);
  int nRead = static_cast<int>(sceNetRecvfrom(socket_,
    reinterpret_cast<char*>(receiving_vector.data()),
    receiving_vector.size(), 0, (SceNetSockaddr *)&addrinfo_res_,
    (SceNetSocklen_t *)&len));

  /*
  if (nRead == 0) {
    DebugUtils::Log("Socket [%d] closed.", socket_);
  }
  */

  if (nRead < 0) {
    recv_handler_(true, sce_net_errno, "sceNetRecvfrom() failed", nRead, receiving_vector);
    CloseSocket();
  }
  else {
    recv_handler_(false, 0, "", nRead, receiving_vector);
  }
}


bool FunapiUdpImpl::Send(const fun::vector<uint8_t> &body, const SendCompletionHandler &send_completion_handler) {
  uint8_t *buf = const_cast<uint8_t*>(body.data());

  int nSent = static_cast<int>(sceNetSendto(socket_,
    reinterpret_cast<char*>(buf),
    body.size(), 0,
    (SceNetSockaddr *)&addrinfo_res_,
    sizeof(addrinfo_res_)));

  /*
  if (nSent == 0) {
    DebugUtils::Log("Socket [%d] closed.", socket_);
  }
  */

  if (nSent < 0) {
    send_completion_handler(true, sce_net_errno, "sceNetSendto() failed", nSent);
    CloseSocket();
  }
  else {
    send_completion_handler(false, 0, "", nSent);
  }

  return true;
}


////////////////////////////////////////////////////////////////////////////////
// FunapiSocket implementation.

bool FunapiSocket::Select() {
  return FunapiSocketImpl::Select();
}


////////////////////////////////////////////////////////////////////////////////
// FunapiAddrInfo implementation.

FunapiAddrInfo::FunapiAddrInfo()
  : impl_(std::make_shared<FunapiAddrInfoImpl>()) {
}


FunapiAddrInfo::~FunapiAddrInfo() {
  // DebugUtils::Log("%s", __FUNCTION__);
}


std::shared_ptr<FunapiAddrInfo> FunapiAddrInfo::Create() {
  return std::make_shared<FunapiAddrInfo>();
}


fun::string FunapiAddrInfo::GetString() {
  return impl_->GetString();
}


std::shared_ptr<FunapiAddrInfoImpl> FunapiAddrInfo::GetImpl() {
  return impl_;
}


////////////////////////////////////////////////////////////////////////////////
// FunapiTcp implementation.

FunapiTcp::FunapiTcp()
: impl_(std::make_shared<FunapiTcpImpl>()) {
  FunapiSocketImpl::Add(impl_);
}


FunapiTcp::~FunapiTcp() {
  // DebugUtils::Log("%s", __FUNCTION__);
}


std::shared_ptr<FunapiTcp> FunapiTcp::Create() {
  return std::make_shared<FunapiTcp>();
}


void FunapiTcp::Connect(const char* hostname_or_ip,
                        const int port,
                        const time_t connect_timeout_seconds,
                        const bool disable_nagle,
                        const ConnectCompletionHandler &connect_completion_handler,
                        const SendHandler &send_handler,
                        const RecvHandler &recv_handler) {
  impl_->Connect(hostname_or_ip,
                port,
                connect_timeout_seconds,
                disable_nagle,
                connect_completion_handler,
                send_handler,
                recv_handler);
}


void FunapiTcp::Connect(const char* hostname_or_ip,
                        const int port,
                        const time_t connect_timeout_seconds,
                        const bool disable_nagle,
                        const bool use_tls,
                        const fun::string &cert_file_path,
                        const ConnectCompletionHandler &connect_completion_handler,
                        const SendHandler &send_handler,
                        const RecvHandler &recv_handler) {
  impl_->Connect(hostname_or_ip,
                 port,
                 connect_timeout_seconds,
                 disable_nagle,
                 use_tls,
                 cert_file_path,
                 connect_completion_handler,
                 send_handler,
                 recv_handler);
}


void FunapiTcp::Connect(std::shared_ptr<FunapiAddrInfo> info,
                        const ConnectCompletionHandler &connect_completion_handler) {
  impl_->Connect(info->GetImpl()->GetAddrInfo(), connect_completion_handler);
}


bool FunapiTcp::Send(const fun::vector<uint8_t> &body, const SendCompletionHandler &send_handler) {
  return impl_->Send(body, send_handler);
}


int FunapiTcp::GetSocket() {
  return impl_->GetSocket();
}


void FunapiTcp::OnSelect(const fd_set rset, const fd_set wset, const fd_set eset) {
  impl_->OnSelect(rset, wset, eset);
}

////////////////////////////////////////////////////////////////////////////////
// FunapiUdp implementation.

FunapiUdp::FunapiUdp(const char* hostname_or_ip,
                     const int port,
                     const InitHandler &init_handler,
                     const SendHandler &send_handler,
                     const RecvHandler &recv_handler)
: impl_(std::make_shared<FunapiUdpImpl>(hostname_or_ip,
                                        port,
                                        init_handler,
                                        send_handler,
                                        recv_handler)) {
  FunapiSocketImpl::Add(impl_);
}


FunapiUdp::~FunapiUdp() {
}


std::shared_ptr<FunapiUdp> FunapiUdp::Create(const char* hostname_or_ip,
                                             const int port,
                                             const InitHandler &init_handler,
                                             const SendHandler &send_handler,
                                             const RecvHandler &recv_handler) {
  return std::make_shared<FunapiUdp>(hostname_or_ip,
                                     port,
                                     init_handler,
                                     send_handler,
                                     recv_handler);
}


bool FunapiUdp::Send(const fun::vector<uint8_t> &body, const SendCompletionHandler &send_handler) {
  return impl_->Send(body, send_handler);
}


int FunapiUdp::GetSocket() {
  return impl_->GetSocket();
}


void FunapiUdp::OnSelect(const fd_set rset, const fd_set wset, const fd_set eset) {
  impl_->OnSelect(rset, wset, eset);
}

}  // namespace fun

#endif