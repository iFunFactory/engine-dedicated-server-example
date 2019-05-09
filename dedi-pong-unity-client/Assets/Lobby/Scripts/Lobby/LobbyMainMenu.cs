using UnityEngine;
using UnityEngine.UI;
using System.Collections;
using Fun;

namespace Prototype.NetworkLobby
{
    //Main menu, mainly only a bunch of callback called by the UI (setup throught the Inspector)
    public class LobbyMainMenu : MonoBehaviour
    {
        public LobbyManager lobbyManager;

        public RectTransform lobbyServerList;
        public RectTransform lobbyPanel;

        public InputField addressInput;
        public InputField portInput;
        public InputField userIdInput;
        public InputField matchNameInput;
        public LobbyInfoPanel infoPanel;


        public void OnEnable()
        {
            lobbyManager.topPanel.ToggleVisibility(true);

            userIdInput.onEndEdit.RemoveAllListeners();
            userIdInput.onEndEdit.AddListener(onEndEditUserId);

            matchNameInput.onEndEdit.RemoveAllListeners();
            matchNameInput.onEndEdit.AddListener(onEndEditGameName);

            if (PlayerPrefs.HasKey("server_ip"))
                addressInput.text = PlayerPrefs.GetString("server_ip");

            if (PlayerPrefs.HasKey("server_port"))
                portInput.text = PlayerPrefs.GetString("server_port");

            if (PlayerPrefs.HasKey("user_id"))
                userIdInput.text = PlayerPrefs.GetString("user_id");
        }

        public void OnClickHost()
        {
            FunDebug.Log("OnClickHost()");
            lobbyManager.StartHost();
        }

        public void OnClickJoin()
        {
            FunDebug.Log("OnClickJoin()");
            lobbyManager.networkAddress = addressInput.text;

            StartJoin();
        }

        public void OnClickLogin()
        {
            bool not_ready = string.IsNullOrEmpty(addressInput.text) || string.IsNullOrEmpty(portInput.text) || string.IsNullOrEmpty(userIdInput.text);
            if (not_ready)
            {
                infoPanel.infoText.fontSize = 30;
                infoPanel.Display("Please fill in the required fields.", "close", null);
                return;
            }

            lobbyManager.networkAddress = addressInput.text;
            PlayerPrefs.SetString("server_ip", addressInput.text);
            PlayerPrefs.SetString("server_port", portInput.text);
            PlayerPrefs.SetString("user_id", userIdInput.text);

            ushort port = ushort.Parse(portInput.text);
            PongManager.instance.StartClient(addressInput.text, port, userIdInput.text);

            lobbyManager.SetServerInfo("Connecting...", addressInput.text);
        }

        public void StartJoin()
        {
            lobbyManager.ChangeTo(lobbyPanel);

            lobbyManager.StartClient();

            lobbyManager.backDelegate = lobbyManager.StopClientClbk;
            lobbyManager.DisplayIsConnecting();

            lobbyManager.SetServerInfo("Connecting...", lobbyManager.networkAddress);
        }

        public void OnLoginFailed()
        {
            infoPanel.Display("Login failed.", "close", () => {  lobbyManager.SetServerInfo("Offile", ""); });
            return;
        }

        public void OnClickDedicated()
        {
            lobbyManager.ChangeTo(null);
            lobbyManager.StartServer();

            lobbyManager.backDelegate = lobbyManager.StopServerClbk;

            lobbyManager.SetServerInfo("Dedicated Server", lobbyManager.networkAddress);
        }

        public void OnClickCreateMatchmakingGame()
        {
            lobbyManager.StartMatchMaker();
            lobbyManager.matchMaker.CreateMatch(
                matchNameInput.text,
                (uint)lobbyManager.maxPlayers,
                true,
                "", "", "", 0, 0,
                lobbyManager.OnMatchCreate);

            lobbyManager.backDelegate = lobbyManager.StopHost;
            lobbyManager._isMatchmaking = true;
            lobbyManager.DisplayIsConnecting();

            lobbyManager.SetServerInfo("Matchmaker Host", lobbyManager.matchHost);
        }

        public void OnClickOpenServerList()
        {
            lobbyManager.StartMatchMaker();
            lobbyManager.backDelegate = lobbyManager.SimpleBackClbk;
            lobbyManager.ChangeTo(lobbyServerList);
        }

        void onEndEditUserId(string text)
        {
            if (Input.GetKeyDown(KeyCode.Return))
            {
                OnClickLogin();
            }
        }

        void onEndEditGameName(string text)
        {
            if (Input.GetKeyDown(KeyCode.Return))
            {
                OnClickCreateMatchmakingGame();
            }
        }
    }
}
