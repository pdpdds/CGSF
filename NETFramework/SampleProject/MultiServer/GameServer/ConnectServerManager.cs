using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace GameServer
{
    class ConnectServerManager
    {
        Dictionary<int, ConnectServer> ServerSessionMap = new Dictionary<int, ConnectServer>();
        

        
        public ERROR_CODE AddServer(int sessionID, string name)
        {
            if (ServerSessionMap.ContainsKey(sessionID))
            {
                return ERROR_CODE.ADD_SERVER_DUPLICATION_SESSION;
            }

            var server = new ConnectServer();
            server.SetInfo(sessionID, name);

            ServerSessionMap.Add(sessionID, server);
            
            return ERROR_CODE.NONE;
        }

        public void RemoveServer(int serial)
        {
            var user = GetServer(serial);

            if (user != null)
            {
                ServerSessionMap.Remove(serial);
            }
        }

        public ConnectServer GetServer(int serial)
        {
            ConnectServer user = null;
            ServerSessionMap.TryGetValue(serial, out user);
            return user;
        }
    }

    class ConnectServer
    {
        public int SessionID { get; private set; }
        public string Name { get; private set; }
        
        public void SetInfo(int sessionID, string name)
        {
            SessionID = sessionID;
            Name = name;
        }

       
    }
}
