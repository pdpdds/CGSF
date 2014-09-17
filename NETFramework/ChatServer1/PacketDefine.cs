using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace ChatServer1
{
    enum PACKET_ID : ushort
    {
        REQUEST_LOGIN   = 1,
        RESPONSE_LOGIN  = 2,

        REQUEST_CHAT    = 11,
        NOTICE_CHAT     = 12,
    }

    enum REQUEST_RESULT : ushort
    {
        ERROR   = 0,


        NONE    = UInt16.MaxValue
    }

    struct JsonPacketRequestLogin
    {
        public string ID;
        public string PW;
    }

    struct JsonPacketResponseLogin
    {
        public UInt16 Result;
    }


    struct JsonPacketRequestChat
    {
        public string chat;
    }

    struct JsonPacketNoticeChat
    {
        public int packetID;
        public string who;
        public string chat;
    }
}
