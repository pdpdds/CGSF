using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace ChatServer1
{
    class LobbyManager
    {
        ServerNetwork ServerNetworkRef;
        List<Lobby> LobbyList = new List<Lobby>();


        public void CreateLobby(ServerNetwork serverNetwork, int lobbyCount, int maxUserCount)
        {
            ServerNetworkRef = serverNetwork;

            for (var i = 0; i < lobbyCount; ++i)
            {
                var lobyIndex = (short)(i + 1);

                var lobby = new Lobby();

                lobby.Init(lobyIndex, maxUserCount);

                LobbyList.Add(lobby);
            }
        }

        public ERROR_CODE EnterLobby(short lobbyID, ConnectUser user)
        {
            var error = ERROR_CODE.NONE;

            var lobby = LobbyList.Find(x => x.ID == lobbyID);
            if (lobby == null)
            {
                return ERROR_CODE.ENTER_LOBBY_INVALID_LOBBY_ID;
            }

            error = lobby.AddUser(user);
            if (error == ERROR_CODE.NONE)
            {
                user.EnterLobby(lobbyID);
            }

            return error;
        }

        public ERROR_CODE LeaveLobby(short lobbyID, string userID)
        {
            var lobby = LobbyList.Find(x => x.ID == lobbyID);
            if (lobby == null)
            {
                return ERROR_CODE.LEAVE_LOBBY_NO_LOBBY;
            }

            lobby.RemoveUser(userID);
            
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
