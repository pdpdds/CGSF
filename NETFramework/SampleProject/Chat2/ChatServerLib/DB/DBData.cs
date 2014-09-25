using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

using CSCommonLib;

namespace ChatServerLib.DB
{
    public class RequestData
    {
        public CSCommonLib.PACKET_ID PacketID;
        public int SessionID;
        public short PacketProcessIndex;
        public string UserID;
        public byte[] Datas;
    }

    public class ResponseData
    {
        public CSCommonLib.PACKET_ID PacketID;
        public int SessionID;
        public short PacketProcessIndex;
        public byte[] Datas;
    }

    public class RequestLogin
    {
        public string PW;
    }

    public class ResponseLogin
    {
        public ERROR_CODE Result;
        public string UserID;
    }
}
