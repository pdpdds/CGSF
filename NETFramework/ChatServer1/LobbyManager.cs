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
                var lobyIndex = i + 1;

                var lobby = new Lobby();

                lobby.Init(lobyIndex, maxUserCount);

                LobbyList.Add(lobby);
            }
        }
    }
}
