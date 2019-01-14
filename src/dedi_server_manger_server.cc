// Copyright (C) 2018 iFunFactory Inc. All Rights Reserved.
//
// This work is confidential and proprietary to iFunFactory Inc. and
// must not be used, disclosed, copied, or distributed without the prior
// consent of iFunFactory Inc.

#include <boost/bind.hpp>
#include <funapi.h>
#include <gflags/gflags.h>

#include <src/dedi_server_manger_object.h>

#include <src/bot/simple_bot_client.h>
#include <src/dsm/dedi_server_helper.h>
#include <src/dsm/matchmaking_server_wrapper.h>
#include <src/dsm/message_handler.h>

// You can differentiate game server flavors.
// You can see more details in the following link.
// https://www.ifunfactory.com/engine/documents/tutorial/ko/flavor.html
// 설정한 서버 flavor 는 이 플래그로 구분할 수 있습니다.
// 보다 자세한 사항은 아래 링크를 참고해주세요.
// https://www.ifunfactory.com/engine/documents/tutorial/ko/flavor.html
DECLARE_string(app_flavor);

// 'enable_redis' flag of Redis section in your MANIFEST.json
// MANIFEST.json 에 있는 'enable_redis' 플래그
DECLARE_bool(enable_redis);


// 봇 클라이언트 개수를 지정합니다.
// dedi_server_manger.bot-local -bot_clients=<클라이언트 개수> 로 지정할 수
// 있습니다.
DEFINE_int64(bot_clients, 1, "Number of clients that are used for test.");


namespace {

//
// DedicatedServerManagerExampleServer 컴포넌트
//
// 아이펀 엔진이 실행할 컴포넌트를 정의하는 곳입니다.
// 1. 프로젝트 생성 후 개발자가 만들어야 할 모든 코드는 src/dsm 에 있습니다.
// 2. bot 클라이언트가 사용할 코드는 src/bot 에 있습니다.
//
class DediServerMangerServer : public Component {
 public:
  static bool Install(const ArgumentMap &arguments) {
    LOG(INFO) << "Built using Engine version: " << FUNAPI_BUILD_IDENTIFIER;

    //
    // 컴포넌트 초기화 단계
    //
    // 초기화 단계는 엔진을 구성하는 다른 컴포넌트들이 아직 시작(Start)하지 않은
    // 상태입니다. 이 곳에서는 핸들러 또는 객체 초기화 등의 작업만 두는 게 좋습니다.
    //

    // Kickstarts the Engine's ORM.
    // Do not touch this, unless you fully understand what you are doing.
    // 엔진 ORM 초기화 작업을 진행합니다.
    // 이 코드가 하는 역할을 명확하게 알기 전까지는 삭제하지 않고 두는 게 좋습니다.
    dedi_server_manger::ObjectModelInit();

    // Redis must be enabled for Dedicated Server Manager.
    // Please see 'redis_servers' of Redis section in your MANIFEST.json
    // For more details.
    // 데디케이티드 서버 매니저를 사용하기 위해서는 Redis 가 필요합니다.
    // 보다 자세한 사항은 MANIFEST.json 파일 안의 Redis 항목에 있는 'redis_servers'
    // 를 참고해주세요.
    LOG_ASSERT(FLAGS_enable_redis);

    // 클라이언트 요청 핸들러를 등록합니다.
    if (FLAGS_app_flavor == "server") {
      dsm::RegisterMessageHandler();
      dsm::DediServerHelper::RegisterHandler();
    } else {
      LOG_ASSERT(FLAGS_app_flavor == "bot");
      bot::SimpleBotClient::Install(4 /* 스레드 개수 */, 2 /* 클라이언트 개수 */);
    }

    return true;
  }

  static bool Start() {
    //
    // 컴포넌트 실행 단계
    //
    // 엔진의 다른 모든 컴포넌트를 실행항 후 이 컴포넌트의 실행(Start) 함수를
    // 호출합니다. 실행 단계에서는 데이터베이스 쿼리 또는 타이머 실행 등 서버 동작에
    // 필수적인 기능들을 추가하는 것이 좋습니다.
    //

    if (FLAGS_app_flavor == "server") {
      dsm::MatchmakingServerWrapper::Start();
    } else {
      LOG_ASSERT(FLAGS_app_flavor == "bot");
      // 봇 클라이언트를 실행합니다.
      bot::SimpleBotClient::Start();
    }

    return true;
  }

  static bool Uninstall() {
    //
    // 컴포넌트 종료 단계
    //
    // 다른 엔진 컴포넌트를 종료하기 전 이 컴포넌트의 종료(Uninstall) 함수를 호출합니다.
    // 이 곳에서는 서버를 안전하게 종료하기 위한 작업(실행 중인 타이머 취소 등)들을
    // 추가해야 합니다. 서버가 알 수 없는 이유로 충돌한 경우에도 이 함수를 호출하나
    // 항상 보장하는 것은 아니므로 주의해야 합니다.
    //

    if (FLAGS_app_flavor == "server") {
    } else {
      LOG_ASSERT(FLAGS_app_flavor == "bot");
      // 봇 클라이언트 실행을 종료합니다.
      bot::SimpleBotClient::Uninstall();
    }

    return true;
  }
};

}  // unnamed namespace


REGISTER_STARTABLE_COMPONENT(DediServerMangerServer, DediServerMangerServer)
