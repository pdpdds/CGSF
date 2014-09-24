using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace ChatServerLib.DB
{
    public class RequestData
    {
        public CSCommonLib.PACKET_ID PacketID;
        public string SessionID;
        public short PacketProcessIndex;
        public string UserID;
        public byte[] Datas;
    }

    public class ResponseData
    {
        public CSCommonLib.PACKET_ID PacketID;
        public string SessionID;
        public short PacketProcessIndex;
        public byte[] Datas;
    }
}
