using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace GameServer
{
    class PktBaseHandler
    {
        protected ServerNetwork ServerNetworkRef;
        protected ConnectServerManager ServerManagerRef;
        

        public void Init(ServerNetwork serverNetwork, ConnectServerManager serverManager)
        {
            ServerNetworkRef = serverNetwork;
            ServerManagerRef = serverManager;
        }

        
    }
}
