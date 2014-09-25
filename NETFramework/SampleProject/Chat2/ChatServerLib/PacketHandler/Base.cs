using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

using CSCommonLib;

namespace ChatServerLib.PacketHandler
{
    class Base
    {
        protected ServerNetwork ServerNetworkRef;
        protected DB.DBManager DBManagerRef;
        
                
        public void InsertDBRequest(PACKET_ID packetID, int sessionID, string userID, byte[] jobDatas)
        {
            var request = new DB.RequestData()
            {
                PacketID = packetID,
                SessionID = sessionID,
                UserID = userID,
                Datas = jobDatas
            };

            DBManagerRef.InsertRequest(request);
        }

        
    }
}
