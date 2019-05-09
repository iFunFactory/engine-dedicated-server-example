퐁 게임과 유니티 데디케이티드 서버
==============================

이 프로젝트는 아이펀 엔진 서버와 유니티 데디케이티드 서버를 연동해서 사용하는 방법을 보여주는 예제입니다.
클라이언트와 유니티 데디케이티드 서버는 유니티에서 제공하는 Standard Assets의 Lobby Manager에 Pong
게임을 더해 구현하였습니다.

## 아이펀 서버

``아이펀 엔진 서버`` 와 ``유니티 데디케이티드 서버`` 는 직접 연결해 통신하지 않으며 이 둘을 이어주는
``호스트 매니저`` (python server) 가 있습니다.

아이펀 서버는 호스트 매니저와 통신하고 유니티 데디케이티드 서버도 호스트 매니저와 통신을 통해 유저 정보와 게임
데이터를 주고 받게 됩니다. 호스트 매니저는 파이선으로 되어 있으며 유니티 데디케이티드 서버와의 통신은 HTTP 를
사용합니다.

### 서버 연동의 흐름
1. 아이펀 엔진 서버와 호스트 매니저 실행 (호스트 매니저는 처음 실행할 때 유니티 데디케이티드 서버 버전 확인)
2. 클라이언트가 아이펀 엔진 서버에 접속 -> 아이펀 엔진 서버가 호스트 매니저로 유니티 데디케이티드 서버 생성 요청
3. 유니티 데디케이티드 서버 생성 -> 호스트 매니저가 아이펀 엔진 서버로 데디케이티드 서버 정보를 전달 -> 클라이언트에 전달
4. 클라이언트는 아이펀 엔진 서버로부터 데디케이티드 서버 정보를 받아서 데디케이티드 서버에 접속 -> 게임 시작
5. 유니티 데디케이티드 서버가 호스트 매니저로 게임 결과를 전달 -> 아이펀 엔진 서버에 전달

아이펀 엔진 서버와 호스트 매니저의 설정은 아래 링크를 참고해주세요.

<http://www.ifunfactory.com/engine/documents/reference/ko/dedicated-server.html>


## 유니티 데디케이티드 서버

유니티 클라이언트 플러그인은 ``Funapi`` 폴더에 있습니다. 파일들 중 ``FunapiDedicatedServer``
가 호스트 매니저와 통신할 때 사용하는 클래스입니다.

### FunapiDedicatedServer 클래스

아래는 ``FunapiDedicatedServer`` 클래스의 인터페이스에 대한 설명입니다.

```
// 정적 멤버 함수

// 처음 유니티 데디케이티드 서버 실행 후 이 함수를 호출해야 합니다.
// 데디케이티드 서버 초기화를 시작합니다. 초기화 과정은 버전 확인 및 프로세스 인자 확인 등을 포합합니다.
// 올바른 버전 형식(x.y.z.r)을 지정하지 않으면 초기화에 실패하며 데디케이티드 서버를 종료합니다.
// 이 함수가 호출되면 게임 데이터를 요청하고 데이터를 받게 되면 콜백 함수가 호출됩니다.
public static void Start (string version)

// 데디케이티드 서버가 유저 입장이 가능한 상태가 되었음을 호스트 매니저에게 알립니다.
// 이 메시지를 보내기 전까지는 유저가 데디케이티드 서버에 입장하지 않습니다.
public static void SendReady (Action<int /*response_code*/, string /*error_desc*/> callback)

// 사용자가 유니티 데디케이티드 서버에 접속했음을 호스트 매니저에게 알립니다.
public static void SendJoined (string uid, Action<int /*response_code*/, string /*error_desc*/> callback)

// 사용자와 유니티 데디케이티드 서버와의 연결이 끊겼음을 호스트 매니저에게 알립니다.
public static void SendLeft (string uid, Action<int /*response_code*/, string /*error_desc*/> callback)

// 게임 상태를 호스트 매니저에게 전송합니다.
// json 문자열로 전송해야 합니다.
public static void SendGameState(string json_string, Action<int, string> callback)

// 사용자 정의 json 데이터를 호스트 매니저에게 전송합니다.
public static void SendCustomCallback (string json_string, Action<int /*response_code*/, string /*error_desc*/> callback)

// 게임 결과를 호스트 매니저에게 전송합니다.
// 결과 값은 json 문자열이어야 하며 {"result":data} 의 형식이어야 합니다.
// data 의 형식은 정해져 있지 않습니다.
public static void SendResult (string json_string, Action<int /*response_code*/, string /*error_desc*/> callback)

// 해당 uid 사용자의 유저 데이터를 반환합니다.
public static string GetUserDataJsonString (string uid)

// 매치 데이터를 반환합니다.
public static string GetMatchDataJsonString()

// 유저가 처음 유니티 데디케이티드 서버에 접속했을 때 해당 유저가 유효한 사용자인지 검사해야 합니다.
// 사용자 정보가 유효한지 확인하고 사용자 정보를 찾을 수 없을 경우 false를 반환합니다.
public static bool AuthUser (string uid, string token)


// 프로퍼티

// 유니티 데디케이티드 서버로 실행되었을 경우 true를 반환합니다.
// 'Start(string version)' 함수를 호출한 뒤에 값이 정해지므로 'Start(string version)' 함수 호출 뒤에 사용 가능합니다.
public static bool isServer;

// 유니티 데디케이티드 서버가 실행중일 경우 true를 반환합니다.
public static bool isActive;

// 유니티 데디케이티드 서버의 포트 번호입니다.
// 이 포트 번호로 클라이언트가 데디케이티드 서버에 접속하게 됩니다.
public static int serverPort;
```

### 초기화

호스트 매니저로부터 받은 CommandLine Argument 를 처리하고 데디케이티드 서버를 시작하기 위해 Start(string version)
함수를 호출해야 합니다.
데디케이티드 서버 초기화 작업을 할 때 버전 값을 입력해야 합니다.

```
FunapiDedicatedServer.Start("1.0.0.1");

if (FunapiDedicatedServer.isServer)
{
    ...
    // 데디케이티드 서버 시작
    StartCoroutine(startServer());
}
```

### 콜백 함수

FunapiDedicatedServer 콜백 함수는 아래와 같은 종류가 있습니다. 데디케이티드서버 상태가 변경되거나 데이터가
업데이트 되는 경우에 대한 콜백 함수를 등록할 수 있습니다.

```
// 데디케이티드 서버 시작 후 호출되는 콜백입니다.
public static event Action StartCallback;

// 유저의 데이터를 전달하는 콜백입니다.
public static event Action<string, string> UserDataCallback;   // uid, json string

// 게임 데이터를 전달하는 콜백입니다.
public static event Action<string> MatchDataCallback;          // json string

// 호스트 매니저와 연결이 끊겼을 경우 호출되는 콜백입니다.
public static event Action DisconnectedCallback;
```

``DisconnectedCallback()`` 의 경우 호스트 매니저와 연결이 끊겼기 때문에 게임 관련 정보들을 추후 반영하기 위해
파일로 저장하거나 접속 중인 사용자들과 연결을 끊고 종료 처리를 할 수 있습니다.

```
FunapiDedicatedServer.DisconnectedCallback += delegate ()
{
    ...
#if UNITY_EDITOR
    UnityEditor.EditorApplication.Exit(0);
#else
    Application.Quit();
#endif
    return;
};
```

### READY 보내기

게임을 시작할 준비가 완료되면 ``SendReady(Action<int /*response_code*/, string /*error_desc*/> callback)`` 함수를
호출해서 호스트 매니저에 준비가 끝났음을 알려야 합니다.
``SendReady(Action<int /*response_code*/, string /*error_desc*/> callback)`` 함수는 준비가 끝났을 때 한 번만 보내야 합니다.

```
FunapiDedicatedServer.SendReady(delegate (int response_code, string error_desc)
{
    ...
});
```

### 유저 인증

클라이언트에서 SendReady 메시지로 uid 와 토큰 정보를 보내면 이 값이 데디케이티드 서버가 갖고 있는 유저
정보와 일치하는지 확인해주는 작업이 필요합니다.

```
ReadyMessage ready = msg.ReadMessage<ReadyMessage>();

// 유저가 보내 온 uid 와 토큰 값이 맞는지 확인하고 맞으면 게임방에 입장합니다.
if (FunapiManager.instance.AuthUser(ready.uid, ready.token))
{
    Player player = findPlayer(msg.conn.connectionId);

    if (player != null)
    {
        player.uid = ready.uid;
        player.token = ready.token;

        ...

        onPlayerJoined(player);

        PongMessage.SendStart(player.connectionId, hostId == player.connectionId);
    }
}
```

### 게임 결과 전송

게임이 종료되면 호스트 매니저로 게임 결과를 전송합니다.

```
Dictionary<string, object> result = new Dictionary<string, object>();
result["winner"] = winnerId;
Dictionary<string, object> json = new Dictionary<string, object>();
json["result"] = result;

string json_string = FunapiMessage.JsonHelper.Serialize(json);
FunapiDedicatedServer.SendResult(json_stringdelegate (int response_code, string error_desc)
{
    ...
});
...
```

### 빌드

유니티에서 데디케이티드 서버를 빌드할 때에는 반드시 환경설정의 Define Symbols 에
`FUNAPI_DEDICATED_SERVER` 를 추가해야 합니다. 이 심볼이 없을 경우에는 데디케이티드 서버 관련
플러그인 코드가 빌드에 포함되지 않습니다.

그러나 클라이언트를 빌드할 때 이 값을 포함해도 문제가 되지는 않습니다. 호스트 매니저에서 데디케이티드 서버를
실행할 때 CommandLine Argument 를 전달하는데 이 값에 `RunDedicatedServer` 가 포함되어 있으면
데디케이티드 서버로 인식하고 이 값이 없으면 클라이언트로 인식하기 때문입니다.

`FUNAPI_DEDICATED_SERVER` 심볼은 단순히 서버 관련 코드를 빌드에 포함할지 말지에 대한 옵션일 뿐,
이 심볼로 데디케이티드 서버와 클라이언트를 구분하지는 않습니다.

배포용 클라이언트를 빌드할 때 `FUNAPI_DEDICATED_SERVER` 심볼을 제외하고 컴파일을 하면
데디케이티드 서버 관련 플러그인 코드가 클라이언트 바이너리에 포함되지 않습니다.

이 심볼은 데디케이티드 서버 코드를 작성할 때 활용하실 수도 있습니다.

## 클라이언트

### 데디케이티드 서버 접속

아이펀 엔진 서버와 접속에 성공하면 `_sc_dedicated_server` 메시지를 받게 됩니다. 메시지 안에
데디케이티드 서버에 대한 접속 정보와 내게 할당된 토큰 값이 들어 있습니다.

메시지를 받으면 데디케이티드 서버에 접속하고 토큰은 나중에 인증을 위해서 저장해 둡니다.

```
Dictionary<string, object> message = body as Dictionary<string, object>;

if (msg_type == "_sc_dedicated_server")
{
    Dictionary<string, object> redirect = message["redirect"] as Dictionary<string, object>;
    string ip = redirect["host"] as string;
    int port = Convert.ToInt32(redirect["port"]);
    token_ = redirect["token"] as string;

    // 접속할 데디케이티드 서버 정보를 세팅합니다.
    LobbyManager.s_Singleton.networkAddress = ip;
    LobbyManager.s_Singleton.networkPort = port;

    ...
}
```

### READY 보내기

클라이언트 초기화가 완료되면 데디케이티드 서버로 내 uid 와 토큰 정보를 보내야 합니다.

```
ReadyMessage msg = new ReadyMessage();
msg.uid = uid;
msg.token = token;

client.Send(PongMsgType.Ready, msg);
```

### 클라이언트 플러그인

클라이언트 플러그인의 최신 버전은 아래 링크에서 받으실 수 있습니다.

<https://github.com/iFunFactory/engine-plugin-unity3d>



## 테스트

테스트를 위해서는 아이펀 엔진 서버와 호스트 매니저가 필요합니다. 아이펀 엔진은 리눅스 계열의 OS 에서
실행해야 하며 호스트 매니저는 파이썬이므로 플랫폼에 제한은 없으나 유니티 데디케이티드 서버를 실행해야 하므로
데디케이티드 서버를 실행할 수 있는 플랫폼에 호스트 매니저를 설치하는 것이 좋습니다.

여기에서는 아이펀 엔진 서버는 우분투, 호스트 매니저와 데디케이티드 서버는 MacOS 에서 실행하는 것에 대해
설명합니다.


### 아이펀 엔진 서버

아이펀 엔진 서버는 아래와 같은 라이브러리가 필요합니다.

```
redis-server zookeeper zookeeperd
```

MANIFEST.json 파일에서 `enable_redis` 값을 true 로 변경하고 `dependency` 에는 아래 항목을
추가합니다.

```
"DedicatedServerManager": {},
```

호스트 매니저가 같은 머신에 있지 않을 경우 `/etc/redis/redis.conf` 에서 redis 의 bind 주소를 0.0.0.0으로 변경해야 합니다.

```
# By default Redis listens for connections from all the network interfaces
# available on the server. It is possible to listen to just one or multiple
# interfaces using the "bind" configuration directive, followed by one or
# more IP addresses.
#
# Examples:
#
# bind 192.168.1.100 10.0.0.1
bind 0.0.0.0
```

서버가 실행되기 전에 먼저 redis 와 zookeeper 서비스를 먼저 실행하고 그 뒤에 아이펀 엔진 서버를
실행하면 됩니다.


### 호스트 매니저

호스트 매니저를 MacOS 에서 실행할 경우 아래와 같은 라이브러리가 필요합니다. 파이썬은 2.7로 설치해 주세요.

```
python gevent flask redis requests python-gflags netifaces cryptography
```

설정 파일 `funapi-dedicated-server-host.flag` 에 유니티 데디케이티드 서버의 실행 파일 경로를
입력해야 합니다. Linux 의 경우 `/etc/funapi-dedicated-server-host/funapi-dedicated-server-host.flag` 에
설정 파일이 있습니다.
`binary_path` 항목에 아래와 같은 형식으로 입력해 주세요.

```
// Windows일 경우
--binary_path=C:\dedi-server-example\server.exe

// MacOS일 경우
--binary_path=/Users/zoo/dedi-server-example/server.app/Contents/MacOS/server

// Linux일 경우
--binary_path=/home/zoo/dedi-server-example/server
```

데디케이티드 서버를 에디터로 실행하고 싶을 경우 아래 Command를 배치 파일로 만들어서 만든 배치 파일
경로를 `binary_path` 에 입력하면 됩니다.

```
// Windows일 경우
"C:\Program Files\Unity\Editor\Unity.exe" -projectPath /Path/to/project/directory -executeMethod FunapiManager.StartPlay %*

// MacOS일 경우
/Applications/Unity/Unity.app/Contents/MacOS/Unity -projectPath /Path/to/project/directory -executeMethod FunapiManager.StartPlay $@
```

그 외에 전용 서버 타입과 레디스 서버 관련 정보를 입력해 줍니다.

```
# Engine type. Possible values are 'ue4' (UnrealEngine4) and 'unity' (Unity).
--engine_type=unity

# Also, you should update redis_host, redis_port to match the redis address.
--redis_host=127.0.0.1
--redis_port=6379

# If you want to bind the specific NIC for clients, update the following lines.
# And you may choose another NIC for inter-server communication.
# eg) You may use eno1 for games, and eno2 for inter-server communication.
# Depends on your OS, NIC name can be varying - eg) eth0, eno1, ens1, enp2s0.
--game_interface=eth0
--restful_interface=eth1
```

실행은 터미널에서 호스트 매니저가 있는 폴더로 이동해서 아래와 같이 실행하면 됩니다.

```
python -m funapi_dedicated_server --flagfile=./funapi-dedicated-server-host.flag
```

### 유니티 데디케이티드 서버

환경설정의 Define Symbols 에 `FUNAPI_DEDICATED_SERVER` 심볼을 추가해서 빌드한 후 호스트
서버의 설정 파일에 작성해 놓은 경로에 파일을 복사해 둡니다.

### 클라이언트

Lobby Scene의 ``Pong Manager`` 에 아이펀 엔진 서버의 주소와 포트 번호를 입력한 후 빌드해서
클라이언트 실행 파일을 만듭니다. 클라이언트는 두 개가 필요하므로 복사해서 두 개를 만들어 둡니다.

### 실행

클라이언트를 실행하면 자동으로 아이펀 엔진 서버에 접속하고 유니티 데디케이티드 서버 정보를 기다립니다.
실행 후 READY 버튼이 나오기 전까지 아무 버튼도 누르지 마세요.

호스트 매니저에서 유니티 데디케이티드 서버를 실행하고 해당 정보를 아이펀 엔진을 통해 클라이언트에게 전달합니다.
이 정보로 클라이언트는 유니티 데디케이티드 서버에 접속한 후 게임 시작 대기 상태가 됩니다.

두 명의 사용자가 모두 READY 버튼을 누르면 게임이 시작됩니다. 슬라이드 바를 움직여 공을 상대에게 보내는
게임입니다. 공을 쳐내지 못 한 사람이 게임에서 지게 됩니다.

게임이 종료되면 데디케이티드 서버가 게임 결과를 호스트 매니저에게 전송합니다. 호스트 매니저는 이 결과를
아이펀 엔진 서버로 전달합니다.


## 도움말

클라이언트 플러그인의 도움말은 <https://www.ifunfactory.com/engine/documents/reference/ko/client-plugin.html> 를 참고해 주세요.

플러그인에 대한 궁금한 점은 <https://answers.ifunfactory.com> 에 질문을 올려주세요.
가능한 빠르게 답변해 드립니다.

그 외에 플러그인에 대한 문의 사항이나 버그 신고는 <funapi-support@ifunfactory.com> 으로 메일을
보내주세요.
