using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace ChatServerLib.Lobby
{
    class LobbyUser
    {
        public string UserID { get; private set; }
        public int SessionID { get; private set; }


        public void Set(int sessionID, string userID)
        {
            UserID = userID;
            SessionID = sessionID;
        }
    }
}
