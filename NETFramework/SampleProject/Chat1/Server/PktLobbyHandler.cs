using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

using CgsfNET64Lib;
using CGSFNETCommon;

namespace ChatServer1
{
    /// <summary>
    /// 로비 관련 핸들러
    /// </summary>
    class PktLobbyHandler : PktBaseHandler
    {
        public void RequestEnterLobby(SFNETPacket packetData, ConnectUser user)
        {
            if (user == null)
            {
                return;
            }

            try
            {
                var request = JsonEnDecode.Decode<JsonPacketRequestEnterLobby>(packetData.GetData());

                // 이미 로비에 들어가 있는가?
                if (user.LobbyID != 0)
                {
                    ResponseEnterLobby(packetData.SessionID(), ERROR_CODE.ENTER_LOBBY_ALREADY_LOBBY, 0);
                    return;
                }

                // 로비에 추가
                var error = LobbyManagerRef.EnterLobby(request.LobbyID, user);
                if (error != ERROR_CODE.NONE)
                {
                    ResponseEnterLobby(packetData.SessionID(), error, 0);
                    return;
                }

                ResponseEnterLobby(packetData.SessionID(), ERROR_CODE.NONE, request.LobbyID);

                InnerMessageQueue.CurrentLobbyUserCount(request.LobbyID, LobbyManagerRef.LobbyCurrentUserCount(request.LobbyID));
            }
            catch (Exception)
            {
                ResponseEnterLobby(packetData.SessionID(), ERROR_CODE.ERROR, 0);
            }
        }

        void ResponseEnterLobby(int sessionID, ERROR_CODE result, short lobbyID)
        {
            if (result == ERROR_CODE.NONE)
            {
                DevLog.Write(string.Format("[EnterLobby] lobbyID:{0}, sessionID:{1}", lobbyID, sessionID), LOG_LEVEL.INFO);
            }
            else
            {
                DevLog.Write(string.Format("[EnterLobby 실패] lobbyID:{0}, sessionID:{1}, result:{2}", lobbyID, sessionID, result.ToString()), LOG_LEVEL.ERROR);
            }

            var response = new JsonPacketResponseEnterLobby() { Result = result, LobbyID = lobbyID };
            ServerNetworkRef.Send<JsonPacketResponseEnterLobby>(sessionID, PACKET_ID.RESPONSE_ENTER_LOBBY, response);
        }


        public void RequestLeaveLobby(SFNETPacket packetData, ConnectUser user)
        {
            if (user == null)
            {
                return;
            }

            try
            {
                var request = JsonEnDecode.Decode<JsonPacketRequestLeaveLobby>(packetData.GetData());

                if (user.LobbyID <= 0 || request.LobbyID != user.LobbyID)
                {
                    ResponseLeaveLobby(packetData.SessionID(), ERROR_CODE.LEAVE_LOBBY_DO_NOT_ENTER_LOBBY, 0);
                    return;
                }

                var result = LobbyManagerRef.LeaveLobby(user.LobbyID, user.ID);
                if (result != ERROR_CODE.NONE)
                {
                    ResponseLeaveLobby(packetData.SessionID(), result, 0);
                    return;
                }

                user.LeaveLobby();

                ResponseLeaveLobby(packetData.SessionID(), ERROR_CODE.NONE, 0);

                InnerMessageQueue.CurrentLobbyUserCount(request.LobbyID, LobbyManagerRef.LobbyCurrentUserCount(request.LobbyID));
            }
            catch (Exception)
            {
                ResponseLeaveLobby(packetData.SessionID(), ERROR_CODE.ERROR, 0);
            }
        }

        void ResponseLeaveLobby(int sessionID, ERROR_CODE result, short lobbyID)
        {
            if (result == ERROR_CODE.NONE)
            {
                DevLog.Write(string.Format("[LeaveLobby] lobbyID:{0}, sessionID:{1}", lobbyID, sessionID), LOG_LEVEL.INFO);
            }
            else
            {
                DevLog.Write(string.Format("[LeaveLobby 실패] lobbyID:{0}, sessionID:{1}, result:{2}", lobbyID, sessionID, result.ToString()), LOG_LEVEL.ERROR);
            }

            var response = new JsonPacketResponseLeaveLobby() { Result = result };
            ServerNetworkRef.Send<JsonPacketResponseLeaveLobby>(sessionID, PACKET_ID.RESPONSE_LEAVE_LOBBY, response);
        }


        public void RequestChat(SFNETPacket packetData, ConnectUser user)
        {
            if (user == null)
            {
                return;
            }

            try
            {
                var request = JsonEnDecode.Decode<JsonPacketRequestChat>(packetData.GetData());

                if (user.LobbyID <= 0)
                {
                    ResponseErrorLobbyChat(packetData.SessionID(), ERROR_CODE.LOBBY_CHAT_DO_NOT_ENTER_LOBBY);
                    return;
                }

                LobbyManagerRef.LobbyChat(user.LobbyID, user.ID, request.Chat);
            }
            catch (Exception)
            {
                ResponseErrorLobbyChat(packetData.SessionID(), ERROR_CODE.ERROR);
            }
        }

        void ResponseErrorLobbyChat(int sessionID, ERROR_CODE result)
        {
            var response = new JsonPacketNoticeChat() { Result = ERROR_CODE.ERROR };
            ServerNetworkRef.Send<JsonPacketNoticeChat>(sessionID, PACKET_ID.NOTICE_CHAT, response);
        }
    }
}
