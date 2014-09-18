using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

using CgsfNET64Lib;
using CGSFNETCommon;

namespace ChatServer1
{
    class PktCommonHandler : PktBaseHandler
    {
        public void RequestLogin(SFNETPacket packetData, ConnectUser user)
        {
            try
            {
                var request = JsonEnDecode.Decode<JsonPacketRequestLogin>(packetData.GetData());

                // 이미 등록된 유저인가? serial, ID
                var result = UserManagerRef.AddUser(packetData.SessionID(), request.ID);

                var response = new JsonPacketResponseLogin() { Result = result };
                Send<JsonPacketResponseLogin>(packetData.SessionID(), PACKET_ID.RESPONSE_LOGIN, response);

                DevLog.Write(string.Format("[Login] result:{0}, UserID:{1}, SessionID{2}", result.ToString(), request.ID, packetData.SessionID()), LOG_LEVEL.INFO);
            }
            catch (Exception)
            {
                var response = new JsonPacketResponseLogin() { Result = ERROR_CODE.ERROR };
                Send<JsonPacketResponseLogin>(packetData.SessionID(), PACKET_ID.RESPONSE_LOGIN, response);
            }
        }
    }
}
