using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

using CgsfNET64Lib;
using CGSFNETCommon;

namespace GameServer
{
    class PktCommonHandler : PktBaseHandler
    {
        public void RequestRegistServer(SFNETPacket packetData)
        {
            try
            {
                var request = JsonEnDecode.Decode<JsonPacketRequestReqistServer>(packetData.GetData());

                // 이미 등록된 유저인가? serial, ID
                var result = ServerManagerRef.AddServer(packetData.SessionID(), request.ServerName);

                var response = new JsonPacketResponseReqistServer() { Result = result, ServerName = "GameServer" };
                ServerNetworkRef.Send<JsonPacketResponseReqistServer>(packetData.SessionID(), PACKET_ID.RESPONSE_REGIST_SERVER, response);

                DevLog.Write(string.Format("[RegistServer] result:{0}, ServerName:{1}, SessionID{2}", result.ToString(), request.ServerName, packetData.SessionID()), LOG_LEVEL.INFO);
            }
            catch
            {
            }
        }
    }
}
