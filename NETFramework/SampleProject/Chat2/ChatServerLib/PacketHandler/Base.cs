using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;



namespace ChatServerLib.PacketHandler
{
    class Base
    {
        protected ServerNetwork ServerNetworkRef;
        protected ConnectUserManager UserManagerRef;
        protected LobbyManager LobbyManagerRef;

        public void Init(ServerNetwork serverNetwork, ConnectUserManager userManager, LobbyManager lobbyManager)
        {
            ServerNetworkRef = serverNetwork;
            UserManagerRef = userManager;
            LobbyManagerRef = lobbyManager;
        }


        
    }
}
