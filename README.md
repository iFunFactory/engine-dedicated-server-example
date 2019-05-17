# iFun Engine 과 데디케이티드 서버 연동 샘플

이 프로젝트는 아이펀엔진의 **DedicatedServerManager** 콤포넌트와 **funapi_ds_host_agent**, 그리고, Unity3d 또는 Unreal Engine 4(이후 UE4) 데디케이티드 서버(이후 데디서버)의 연동 방법에 대한 샘플 프로젝트입니다.  
**iFun Engine** 의 데디서버 지원 기능에 대해서는 [관련 매뉴얼](https://www.ifunfactory.com/engine/documents/reference/ko/dedicated-server.html#dedicated-server)에서 자세히 설명하고 있습니다.

## 프로젝트 구성
이 프로젝트는 다음과 같은 서브 프로젝트들로 구성되어 있습니다.
* **server-app** : 아이펀엔진으로 제작된 게임서버입니다. 엔진의 **DedicatedServerManager** 콤포넌트를 사용합니다.
* **dedi-pong-unity-client** : server-app 과 연동하는 유니티 프로젝트입니다.**funapi-unity-plugin** 을 포함하고 있습니다.
* **dedi-ue4-client** : 곧 공개할 예정입니다.

## 게임 서버 빌드 및 실행하기

게임 서버의 운영체제는 *Ubuntu 16.04 Xenial* 이며, 사용자 홈 디렉터리에 **engine-dedicated-server-example** 프로젝트를 저장 해 두었다고 가정하겠습니다.

### 1. 서버 호스트에 **iFun Engine** 패키지 설치하기

**server-app** 을 빌드하고 실행하기 위해서는 **iFun Engine** 패키지가 설치된 리눅스 호스트가 필요합니다.  
패키지 설치 방법은 [관련 매뉴얼](https://www.ifunfactory.com/engine/documents/reference/ko/install.html#installing-funapi)를 참고 해 주시기 바랍니다.  

### 2. **server-app** 이 사용하는 어플리케이션 설치

* **Redis-server** : 게임 서버가 **funapi_ds_host_agent** 목록을 읽어오는 데에 사용합니다.
* **Zookeeperd** : 게임 서버가 RPC 기능이나 매치메이킹 기능을 사용하기 위해 필요합니다. 본 샘플에서는 매치메이킹 기능을 사용합니다.

### 3. **server-app** 빌드하기

1. **server-app** 디렉터리로 이동해서 다음 명령을 실행하면, 서버의 빌드 디렉터리들을 생성합니다.
`$./dedi_server_manager-source/setup_build_environment --type=makefile`  
위 명령이 정상적으로 진행되면 **dedi_server_manager-build** 디렉터리가 생성됩니다.

2. `dedi_server_manager-build/debug` 디렉터리로 이동해서 *Make* 명령을 실행합니다.  
`$make .`
정상적으로 빌드가 진행되면, 디렉터리 내에 다음과 같은 실행파일들이 생성됩니다.
```
$ ls dedi_server_manger*
dedi_server_manager.bot-launcher
dedi_server_manager.bot-local
dedi_server_manager.server-launcher
dedi_server_manager.server-local
```
서버를 실행하는 데에 사용하는 것은 `dedi_server_manager.server-local` 입니다.

### 4. 설정 확인하기
테스트 서버를 실행하기 전에 설정 파일에 필요한 내용이 정의되어 있는지 확인 해 보겠습니다.  
빌드를 수행한 디렉터리에서 다음 파일을 열어 필요한 콤포넌트들이 올바르게 설정되어 있는지 확인합니다.
`manifests/servers/MANIFEST.json`
아래 내용들을 테스트 환경에 맞게 설정했는지 확인 해 주시기 바랍니다.
```
... 생략 ...
// Redis 설정 게임서버와 데디서버 호스트 매니저는 Redis 를 통해서 서로에 대한 정보를 얻습니다.
// Redis server 가 별도 호스트에서 동작하는 경우 ACL 설정을 미리 확인 해 주시기 바랍니다.
"redis_servers": {
   "": {
     "address": "127.0.0.1:6379",
     "database": 0,
     "auth_pass": ""
   }
},
... 생략 ...
// Zookeeper server 가 별도 호스트에서 동작하는 경우 ACL 설정을 미리 확인 해 주시기 바랍니다.
"ZookeeperClient": {
  "zookeeper_nodes": "localhost:2181",
  "zookeeper_client_count": 4,
  "zookeeper_session_timeout_in_second": 60
},
... 생략 ...
"DedicatedServerManager": {
  "dedicated_server_verbose_log": true,
  "dedicated_server_spawn_timeout": 30
},
... 생략 ...
```

### 5. 실행하기
빌드를 수행한 디렉터리에서 아래 명령을 실행해서 테스트 프로젝트의 게임 서버를 실행합니다.
`$./dedi_server_manager.server-local`
테스트 환경에서는 서비스로 등록하지 않고, *커맨드라인*에서 직접 실행하고, `Ctrl+C` 로 서버 실행을 중지할 수 있습니다.  
다음과 같은 내용이 화면에 표시되면 서버가 정상적으로 동작하는 상태입니다.
```
// 서비스로 실행하지 않기 때문에 로그파일 대신 표준 출력으로 서버 로그를 출력합니다.
// 시간은 무시 해 주시기 바랍니다.
I0515 15:32:00.489723  7615 manifest_handler.cc:790] Starting DediServerManagerServer
I0515 15:32:15.483315  7672 dedicated_server_manager.cc:2473] Run as DSM Master.
I0515 15:32:15.483348  7672 dedicated_server_list_manager.cc:674] Refreshing server list from Redis
I0515 15:32:15.484256  7673 dedicated_server_manager.cc:2987] The spawn-queue is empty, check again after few seconds: seconds=3
I0515 15:32:18.484607  7672 dedicated_server_manager.cc:2987] The spawn-queue is empty, check again after few seconds: seconds=3
I0515 15:32:21.484997  7673 dedicated_server_manager.cc:2987] The spawn-queue is empty, check again after few seconds: seconds=3
I0515 15:32:24.485390  7674 dedicated_server_manager.cc:2987] The spawn-queue is empty, check again after few seconds: seconds=3
I0515 15:32:27.485739  7675 dedicated_server_manager.cc:2987] The spawn-queue is empty, check again after few seconds: seconds=3
// 위와 같은 내용을 주기적으로 출력하는 상태
```

## 데디서버 호스트 준비

**데디서버 호스트** 는 데디서버를 실행 할 호스트이며, **funapi_ds_host_agent** 가 동작하고 있어야 합니다.  
운영 환경에서는 빌드 타겟 운영체제와 서비스 운영체제가 다를 수 있습니다만,
테스트 시에는 편의상 클라이언트 엔진 에디터를 실행하는 호스트를 데디서버 호스트로 사용하겠습니다.

**funapi_ds_host_agent** 는 **iFun Engine** 기반 게임 서버와 *Unity* 또는 *Unreal* 기반 *데디케이티드 서버* 사이에 오가는 메시지를 중계합니다.

게임 서버에서 요청한 데디케티이드 서버를 실행하고, 게임 진행 상태와 게임 결과를 게임 서버로 전달하기 위해 실행하는 프로그램입니다.

### 1. **funapi_ds_host_agent** 를 실행하기 위해서는 몇몇 파이썬 패키지가 필요합니다.
다음 명령어를 참고하여 설치 해 주시기 바랍니다.  
```
$pip install --upgrade pip  // pip 버전이 낮은 경우, 아래 패키지들을 설치할 수 없는 경우가 있습니다.
$pip install gevent flask redis requests python-gflags netifaces cryptography
```
*Windows* 환경에서 *pip* 사용 시, 일부 *phthon* 패키지는 [Microsoft Visual C++ Compiler for Python 2.7](https://www.microsoft.com/en-us/download/details.aspx?id=44266) 가 필요할 수 있습니다.

### 2. **funapi_ds_host_agent** 설치 및 설정하기

**funapi_ds_host_agent** 는 *Python2.7* 로 작성되어 있습니다.  
현재, *Linux* 버전은 *apt* 또는 *yum* 같은 패키지 관리자를 사용해서 서비스 형태로 설치 및 운영할 수 있으며,  
*OSX(Mac)* 또는 *Windows* 그리고, *Linux* 에서도 **python 모듈** 소스코드를 직접 받아서 터미널에서 실행할 수 있습니다.
**python 모듈** 소스코드는 funapi-support@ifunfactory.com 으로 요청 해 주시기 바랍니다.  

* **서비스로 설치하기 (Linux)**
  * Ubuntu
  `sudo apt install funapi1-dedicated-server-host`
  * Centos
  `sudo yum install funapi1-dedicated-server-host` 

* **커맨드라인에서 사용하기 (OSX, Windows, Linux)**
**funapi_ds_host_agent** 패키지를 특정 디렉터리에 압축 해제 하고, 해당 디렉터리로 이동합니다.  
`funapi-dedicated-server-host.flag` 파일의 내용을 다음 설명을 참고하여 테스트 환경에 맞게 수정 해 주세요.  
`Linux` 호스트에서 서비스 패키지로 설치한 경우에는 `/etc/funapi-dedicated-server-host/` 디렉터리에 `funapi-dedicated-server-host.flag` 파일이 존재합니다.
  * **binary_path** : 데디서버 호스트 매니저가 실행할 수 있는 데디서버 실행파일 경로. 이 설정이 가리키는 경로에 실행 가능한 파일이 존재하지 않으면 서버가 정상 동작하지 않습니다.
  * **engine_type** : 데디서버 제작에 사용한 엔진 타입을 **unity** 또는 **ue4** 중에 설정합니다.
  * **redis_host** : Redis 호스트 ip 를 설정합니다.
  * **redis_port** : Redis 호스트의 Redis 서비스 포트를 설정합니다.
  * **game_ip** : **funapi_ds_host_agent** 의 게임 서비스 ip 를 설정합니다. 이 설정이 없는 경우에 한하여 **game_interface** 파라미터를 통해서 네트워크 인터페이스 이름으로 설정할 수 있습니다.
  * **restful_ip** : **funapi_ds_host_agent** 의 RestfulAPI 서비스 ip 를 설정합니다. 이 설정이 없는 경우에 한하여 **restful_interface** 파라미터를 통해서 네트워크 인터페이스 이름을 설정할 수 있습니다.

### 3. 실행하기

* **서비스로 실행하기 (Linux)**

*Windows* 또는 *MacOS* 와 같은 방법으로 소스코드로부터 직접 실행 할 수도 있고, 패키지로 설치한 경우는 서비스로 실행할 수 있습니다.
`$sudo systemctl start funapi-dedicated-server-host`

*Linux* 환경에서 Unreal Engine 4 데디케이티드 서버를 실행하는 경우, 아래 링크를
참고하시어 서비스 파일의 실행 *User* 아이디를 조정 해 주시기 바립니다.
https://www.ifunfactory.com/engine/documents/reference/ko/dedicated-server.html#linux-unreal-engine-4


* **커맨드라인에서 사용하기 (OSX, Windows, Linux)**

**funapi_ds_host_agent** 소스코드 디렉터리의 상위 디렉터리에서 다음과 같은 명령으로 실행 할 수 있습니다.
`$python -m funapi_dedicated_server --flagfile=./funapi_dedicated_server/funapi-dedicated-server-host.flag`


## Unity 데디서버 프로젝트 빌드

### 1. 빌드하기

기본적으로 하나의 빌드 결과물을 가지고, 실행 시 옵션을 전달해서 클라이언트 또는 서버로 동작하도록 설정할 수 있습니다.  
또는 빌드 옵션의 `Server Build` 를 사용하면 헤드리스 모드로 동작하는 데디서버 전용 실행파일을 따로 빌드 할 수도 있습니다.  

1. `Build Settings->Player Settings...->Scripting Define Symbils` 에 `FUNAPI_DEDICATED_SERVER` 가 정의되어 있는지 확인하고 빌드를 진행합니다.
2. 빌드 결과물은 `engine-dedicated-server-example/dedi-pong-unity-client/Build` 디렉토리에 저장하도록 설정 해 두었습니다.  
여러분의 테스트 환경에 맞는 빌드 결과물의 실행 경로를 기억 해 둡니다.  ``````````````````````````````````
만약, 빌드와 실행 시 각각 다른 호스트를 사용한다면 실행 호스트에 빌드 결과물을 저장한 경로를 기억 해 주시면 됩니다.  
예제에서 사용할 경로는 다음과 같습니다.
```
//Windows
C:\engine-dedicated-server-example\dedi-pong-unity-client\Build\Pong.exe

//Mac
/Users/engine-dedicated-server-example/dedi-pong-unity-client/Build/pong.app/Contents/MacOS/pong

//Linux
/home/user/engine-dedicated-server-example/dedi-pong-unity-client/Build/pong_linux.x86_64
```

<!--
## UE4 데디서버 프로젝트 빌드

### 클라이언트 빌드

### 서버 빌드
-->
