using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace GateWayServer
{
    enum PACKET_ID : ushort
    {
        SYSTEM_DISCONNECTD = 12,

        
        CLIENT_RANGE_FIRST  = 1000,

        REQUEST_LOGIN   = 1001,
        RESPONSE_LOGIN  = 1002,

        CLIENT_RANGE_LAST = 2000,


        SERVER_RANGE_FIRST = 10000,

        REQUEST_REGIST_SERVER   = 10001,
        RESPONSE_REGIST_SERVER  = 10002,

        SERVER_RANGE_LAST = 20000,
    }

    enum ERROR_CODE : ushort
    {
        NONE    = 0,

        ERROR   = 1001,

        ADD_SERVER_DUPLICATION_SESSION = 1011,
    }


#pragma warning disable 649
    struct JsonPacketRequestLogin
    {
        public string ID;
    }

    struct JsonPacketResponseLogin
    {
        public ERROR_CODE Result;
    }


    struct JsonPacketRequestReqistServer
    {
        public string ServerName;
    }

    struct JsonPacketResponseReqistServer
    {
        public ERROR_CODE Result;
        public string ServerName;
    }
#pragma warning restore 649
   
}
