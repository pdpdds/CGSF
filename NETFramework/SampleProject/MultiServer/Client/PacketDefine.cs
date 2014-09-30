using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace ChatClient1
{
    enum PACKET_ID : ushort
    {
        SYSTEM_DISCONNECTD = 12,

        CLIENT_RANGE_FIRST = 1000,

        REQUEST_LOGIN = 1001,
        RESPONSE_LOGIN = 1002,

        CLIENT_RANGE_LAST = 2000,
    }

    enum ERROR_CODE : ushort
    {
        NONE = 0,

        ERROR = 1001,
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
#pragma warning restore 649

    
}
