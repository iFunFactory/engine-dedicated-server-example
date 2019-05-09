using Fun;
using Prototype.NetworkLobby;
using System;
using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using UnityEngine.Networking;


public class PongManager : MonoBehaviour
{
    // host address & port
    public bool manualTest = false;


    public static void StartPlay ()
    {
#if UNITY_EDITOR
        UnityEditor.EditorApplication.isPlaying = true;
#endif
    }

    void Awake ()
    {
        if (instance_ == null)
        {
            instance_ = this;
            DontDestroyOnLoad(gameObject);
        }
        else if (instance_ != this)
        {
            Destroy(gameObject);
        }
    }

    void Start ()
    {
        if (started_ || manualTest)
            return;

        started_ = true;

        FunapiDedicatedServer.Start("1.0.0.1");

        if (FunapiDedicatedServer.isServer)
        {
            FunapiDedicatedServer.UserDataCallback += onReceivedUserData;
            FunapiDedicatedServer.MatchDataCallback += onReceivedMatchData;
            FunapiDedicatedServer.StartCallback += delegate ()
            {
                FunDebug.Log("Start callback......");
            };
            FunapiDedicatedServer.DisconnectedCallback += delegate ()
            {
                FunDebug.Log("Disconnected callback......");
#if UNITY_EDITOR
                UnityEditor.EditorApplication.Exit(0);
#else
                Application.Quit();
#endif
                return;
            };

            StartCoroutine(startServer());
        }
    }

    IEnumerator startServer ()
    {
        while (LobbyManager.s_Singleton == null)
            yield return new WaitForSeconds(0.1f);

        LobbyManager.s_Singleton.networkPort = FunapiDedicatedServer.serverPort;
        LobbyManager.s_Singleton.StartServer();

        yield return new WaitForSeconds(0.5f);
    }

    public void StartClient (string server_ip, ushort port, string user_id)
    {
        user_id_ = user_id;

        GameObject panel = GameObject.Find("MainPanel");
        if (panel != null)
        {
            mainMenu = panel.GetComponent<LobbyMainMenu>();
        }

        session_ = FunapiSession.Create(server_ip);
        session_.TransportEventCallback += onTransportEvent;
        session_.ReceivedMessageCallback += onReceived;

        TcpTransportOption option = new TcpTransportOption();
        option.ConnectionTimeout = 10f;

        session_.Connect(TransportProtocol.kTcp, FunEncoding.kJson, port, option);
    }

    public void LogoutClient()
    {
        session_.SendMessage("logout", new Dictionary<string, object>());
    }

    public bool AuthUser (string user_id, string token)
    {
        if (manualTest)
            return true;

        return FunapiDedicatedServer.AuthUser(user_id, token);
    }

    public static void GetUserIdAndToken (out string user_id, out string token)
    {
        user_id = user_id_;
        token = token_;
    }

    void onTransportEvent (TransportProtocol protocol, TransportEventType type)
    {
        if (type == TransportEventType.kStarted)
        {
            Dictionary<string, object> body = new Dictionary<string, object>();
            body["account_id"] = user_id_;
            body["platform"] = "unity";
            session_.SendMessage("login", body);
        }
        else if (type == TransportEventType.kStopped)
        {
            if (signing_out_)
            {
                signing_out_ = false;
                return;
            }

            if (mainMenu != null)
            {
                mainMenu.lobbyManager.DisplayIsError("Disconnected.");
            }
        }
    }

    void onReceived (string msg_type, object body)
    {
        Dictionary<string, object> message = body as Dictionary<string, object>;

        if (msg_type == "_sc_dedicated_server")
        {
            Dictionary<string, object> redirect = message["redirect"] as Dictionary<string, object>;
            string ip = redirect["host"] as string;
            int port = Convert.ToInt32(redirect["port"]);
            token_ = redirect["token"] as string;

            LobbyManager.s_Singleton.networkAddress = ip;
            LobbyManager.s_Singleton.networkPort = port;

            if (mainMenu != null)
            {
                mainMenu.StartJoin();
            }
        }
        else if (msg_type == "login")
        {
            Dictionary<string, object> err_data = message["error"] as Dictionary<string, object>;
            int error_code = Convert.ToInt32(err_data["code"]);
            string error_desc = err_data["message"] as string;

            if (error_code == 200) // success
            {
                // {
                //   "account_id": "id",
                //   "match_type": 1
                //   "user_data": {
                //      "level": 70,
                //      "ranking_score": 1500,
                //      ...
                //   },
                // }
                Dictionary<string, object> match = new Dictionary<string, object>();
                match["account_id"] = user_id_;
                match["match_type"] = 1;
                Dictionary<string, object> user_data = new Dictionary<string, object>();
                user_data["level"] = 1;
                user_data["mmr_score"] = 100;
                match["user_data"] = user_data;

                session_.SendMessage("match", match);
            }
            else
            {
                if (mainMenu != null)
                {
                    mainMenu.lobbyManager.DisplayIsLoginFailed();
                }

                FunDebug.Log("Login failed.({0}, code : {1})", error_desc, error_code);
                session_.Stop();
            }
        }
        else if (msg_type == "logout")
        {
            signing_out_ = true;
            Dictionary<string, object> err_data = message["error"] as Dictionary<string, object>;
            int error_code = Convert.ToInt32(err_data["code"]);
            string error_desc = err_data["message"] as string;

            if (error_desc.ToLower() != "ok")
            {
                if (mainMenu != null)
                {
                    string str = string.Format("logout. {0}", error_desc);
                    mainMenu.lobbyManager.DisplayIsError(str);
                }
            }
            FunDebug.Log("Logout.({0}, code : {1})", error_desc, error_code);
            session_.Stop();
        }
    }

    void onReceivedUserData (string user_id, string json_string)
    {
    }

    void onReceivedMatchData (string json_string)
    {
        if (send_ready_)
            return;

        send_ready_ = true;
        FunapiDedicatedServer.SendReady(delegate (int error_code, string error_desc)
        {
            FunDebug.LogWarning("SendReady cb. code : {0}, desc : {1}", error_code, error_desc);
            if (error_code != 0)
            {
                FunDebug.LogWarning("SendReady error. code : {0}, desc : {1}", error_code, error_desc);
#if UNITY_EDITOR
                UnityEditor.EditorApplication.Exit(0);
#else
                Application.Quit();
#endif
            }
        });
    }

    public string UserId
    {
        get { return user_id_; }
    }


    public static PongManager instance { get { return instance_; } }
    static PongManager instance_ = null;

    LobbyMainMenu mainMenu = null;
    FunapiSession session_ = null;
    bool started_ = false;
    bool send_ready_ = false;
    bool signing_out_ = false;
    static string user_id_ = "";
    static string token_ = "";
}
