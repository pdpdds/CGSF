using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace GateWayServer
{
    class PktBaseHandler
    {
        protected ServerNetwork ServerNetworkRef;
        protected ConnectUserManager UserManagerRef;
        

        public void Init(ServerNetwork serverNetwork, ConnectUserManager userManager)
        {
            ServerNetworkRef = serverNetwork;
            UserManagerRef = userManager;
          
        }

        
    }
}
