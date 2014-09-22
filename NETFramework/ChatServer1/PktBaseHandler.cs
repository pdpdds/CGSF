using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace ChatServer1
{
    class PktBaseHandler
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
