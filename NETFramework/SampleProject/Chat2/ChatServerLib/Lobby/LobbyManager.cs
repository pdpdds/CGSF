using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

using CSCommonLib;

namespace ChatServerLib.Lobby
{
    class LobbyManager
    {
        ServerNetwork ServerNetworkRef;

        List<Lobby> LobbyList = new List<Lobby>();


        public void Init(int firstLobbyID, int lobbyCountPerWorkPacketProcess, ServerAppConfig appConfig, ServerNetwork serverNetwork)
        {
            ServerNetworkRef = serverNetwork;

            for (var i = 0; i < lobbyCountPerWorkPacketProcess; ++i)
            {
                var lobbyID = (short)(i + firstLobbyID);

                var lobby = new Lobby();

                lobby.Init(lobbyID, appConfig.MaxLobbyUserCount);

                LobbyList.Add(lobby);
            }
        }

        public ERROR_CODE EnterLobby(short lobbyID, int sessionID, string userID)
        {
            var error = ERROR_CODE.NONE;

            var lobby = LobbyList.Find(x => x.ID == lobbyID);
            if (lobby == null)
            {
                return ERROR_CODE.ENTER_LOBBY_INVALID_LOBBY_ID;
            }
            
            var user = new LobbyUser();
            user.Set(sessionID, userID);

            error = lobby.AddUser(user);
            return error;
        }

        public ERROR_CODE LeaveLobby(short lobbyID, string userID)
        {
            var lobby = LobbyList.Find(x => x.ID == lobbyID);
            if (lobby == null)
            {
                return ERROR_CODE.LEAVE_LOBBY_NO_LOBBY;
            }

            if (lobby.RemoveUser(userID) == false)
            {
                return ERROR_CODE.LEAVE_LOBBY_DO_NOT_ENTER_LOBBY;
            }

            return ERROR_CODE.NONE;
        }

        public void LobbyChat(short lobbyID, string userID, string chatMsg)
        {
            var lobby = LobbyList.Find(x => x.ID == lobbyID);
            if (lobby == null)
            {
                return;
            }

            lobby.Chatting(ServerNetworkRef, userID, chatMsg);
        }

        public int LobbyCurrentUserCount(short lobbyID)
        {
            var lobby = LobbyList.Find(x => x.ID == lobbyID);
            if (lobby == null)
            {
                return 0;
            }

            return lobby.CurrentUserCount();
        }

    }
}
