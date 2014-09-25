using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

using CSCommonLib;
using CGSFNETCommon;

namespace ChatServerLib.PacketHandler
{
    partial class Lobby : Base
    {
        void ResponseEnterLobby(ERROR_CODE result, short lobbyID, string userID)
        {
            if (result == ERROR_CODE.NONE)
            {
                DevLog.Write(string.Format("[EnterLobby] lobbyID:{0}, UserID:{1}", lobbyID, userID), LOG_LEVEL.INFO);
            }
            else
            {
                DevLog.Write(string.Format("[EnterLobby 실패] lobbyID:{0}, UserID:{1}, result:{2}", lobbyID, userID, result.ToString()), LOG_LEVEL.ERROR);
            }


            var jsonData = new JsonPacketInnerNotifyEnterLobby() { Result = result, LobbyID = lobbyID, UserID = userID };
            var bodyData = CGSFNETCommon.JsonEnDecode.Encode<JsonPacketInnerNotifyEnterLobby>(jsonData);

            var InnerPacket = new CgsfNET64Lib.SFNETPacket();
            InnerPacket.SetData(-1, (ushort)PACKET_ID.INNER_NOTIFY_ENTER_LOBBY, bodyData);
            RelayPacketPacketProcess.RelayPacket(0, InnerPacket);
        }


        void ResponseLeaveLobby(ERROR_CODE result, short lobbyID, string userID)
        {
            if (result == ERROR_CODE.NONE)
            {
                DevLog.Write(string.Format("[LeaveLobby] lobbyID:{0}, UserID:{1}", lobbyID, userID), LOG_LEVEL.INFO);
            }
            else
            {
                DevLog.Write(string.Format("[LeaveLobby 실패] lobbyID:{0}, UserID:{1}, result:{2}", lobbyID, userID, result.ToString()), LOG_LEVEL.ERROR);
            }

            var jsonData = new JsonPacketInnerNotifyLeaveLobby() { Result = result, LobbyID = lobbyID, UserID = userID };
            var bodyData = CGSFNETCommon.JsonEnDecode.Encode<JsonPacketInnerNotifyLeaveLobby>(jsonData);

            var InnerPacket = new CgsfNET64Lib.SFNETPacket();
            InnerPacket.SetData(-1, (ushort)PACKET_ID.INNER_NOTIFY_LEAVE_LOBBY, bodyData);
            RelayPacketPacketProcess.RelayPacket(0, InnerPacket);
        }


        //void ResponseErrorLobbyChat(int sessionID, ERROR_CODE result)
        //{
        //    var response = new JsonPacketNoticeChat() { Result = ERROR_CODE.ERROR };
        //    ServerNetworkRef.Send<JsonPacketNoticeChat>(sessionID, PACKET_ID.NOTICE_CHAT, response);
        //}
    }
}
