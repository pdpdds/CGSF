using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace ChatServer1
{
    /// <summary>
    /// 핸들러 클래스의 기초
    /// </summary>
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
