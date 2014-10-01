using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

using CgsfNET64Lib;

namespace GateWayServer
{
    class ServerNetwork : CgsfNET64
    {
        public bool Send<T>(int sessionID, PACKET_ID packetID, T data)
        {
            var bodyData = CGSFNETCommon.JsonEnDecode.Encode<T>(data);
            var result = SendPacket(sessionID, (ushort)packetID, bodyData);
            return result;
        }
    }
}
