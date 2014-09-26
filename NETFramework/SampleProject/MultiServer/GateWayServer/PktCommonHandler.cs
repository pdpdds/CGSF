using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

using CgsfNET64Lib;
using CGSFNETCommon;

namespace GateWayServer
{
    class PktCommonHandler : PktBaseHandler
    {
        public void ResponseRegistServer(SFNETPacket packetData)
        {
            try
            {
                var request = JsonEnDecode.Decode<JsonPacketResponseReqistServer>(packetData.GetData());


                DevLog.Write(string.Format("[ResponseRegistServer] result:{0}, ServerName:{1}, SessionID{2}", request.Result.ToString(), request.ServerName, packetData.SessionID()), LOG_LEVEL.INFO);
            }
            catch
            {
            }
        }


        public void RequestLogin(SFNETPacket packetData, ConnectUser user)
        {
            try
            {
                var request = JsonEnDecode.Decode<JsonPacketRequestLogin>(packetData.GetData());

                // 이미 등록된 유저인가? serial, ID
                var result = UserManagerRef.AddUser(packetData.SessionID(), request.ID);

                var response = new JsonPacketResponseLogin() { Result = result };
                ServerNetworkRef.Send<JsonPacketResponseLogin>(packetData.SessionID(), PACKET_ID.RESPONSE_LOGIN, response);

                DevLog.Write(string.Format("[Login] result:{0}, UserID:{1}, SessionID{2}", result.ToString(), request.ID, packetData.SessionID()), LOG_LEVEL.INFO);
            }
            catch
            {
            }
        }
    }
}
