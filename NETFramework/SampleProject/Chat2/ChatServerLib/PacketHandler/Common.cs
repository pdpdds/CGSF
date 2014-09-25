using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

using MsgPack.Serialization;
using CgsfNET64Lib;
using CGSFNETCommon;
using CSCommonLib;

namespace ChatServerLib.PacketHandler
{
    class Common : Base
    {
        ConnectUserManager UserManagerRef;

        public void Init(ServerNetwork serverNetwork, DB.DBManager dbManager, ConnectUserManager userManager)
        {
            ServerNetworkRef = serverNetwork;
            DBManagerRef = dbManager;
            UserManagerRef = userManager;
        }

        public void SystemClientConnect(SFNETPacket packet)
        {
            var result = UserManagerRef.AddUser(packet.SessionID());
            InnerMessageQueue.CurrentUserCount(UserManagerRef.GetConnectCount());

            if (result == CSCommonLib.ERROR_CODE.NONE)
            {
                DevLog.Write(string.Format("Client Connect. SessionID: {0}", packet.SessionID()), LOG_LEVEL.INFO);
            }
            else
            {
                DevLog.Write(string.Format("Client Connect. SessionID: {0}, Error:{1}", packet.SessionID(), result.ToString()), LOG_LEVEL.ERROR);
            }
        }

        public void SystemClientDisConnected(SFNETPacket packet)
        {
            var user = UserManagerRef.GetUser(packet.SessionID());
            if (user == null)
            {
                return;
            }

            if (user.LobbyID > 0)
            {
                var jsonData = new JsonPacketRequestLeaveLobby() { LobbyID = user.LobbyID, UserID = user.UserID };
                var bodyData = JsonEnDecode.Encode<JsonPacketRequestLeaveLobby>(jsonData);

                var InnerPacket = new CgsfNET64Lib.SFNETPacket();
                InnerPacket.SetData(-1, (ushort)PACKET_ID.REQUEST_LEAVE_LOBBY, bodyData);
                RelayPacketPacketProcess.RelayPacket(user.LobbyID, InnerPacket);
            }


            var result = UserManagerRef.RemoveUser(packet.SessionID());
            InnerMessageQueue.CurrentUserCount(UserManagerRef.GetConnectCount());

            DevLog.Write(string.Format("Client DisConnected. SessionID: {0}", packet.SessionID()), LOG_LEVEL.INFO);
        }

        public void RequestLogin(SFNETPacket packet)
        {
            var user = UserManagerRef.GetUser(packet.SessionID());
            if (user == null)
            {
                return;
            }

            try
            {
                if (user.CurrentState != CONNECT_USER_STATE.NONE)
                {
                    return;
                }

                var request = JsonEnDecode.Decode<CSCommonLib.JsonPacketRequestLogin>(packet.GetData());

                // DB 작업 의뢰한다.
                var dbReqLogin = new DB.RequestLogin() { PW = request.PW };
                var serializer = MessagePackSerializer.Get<DB.RequestLogin>();
                var jobDatas = serializer.PackSingleObject(dbReqLogin);

                InsertDBRequest(PACKET_ID.DB_REQUEST_LOGIN, packet.SessionID(), request.ID, jobDatas);
                
                DevLog.Write("DB에 로그인 요청 보냄", LOG_LEVEL.DEBUG);
            }
            catch(Exception ex)
            {
                DevLog.Write(ex.ToString(), LOG_LEVEL.DEBUG);
            }
        }

        public void DBResponseLogin(SFNETPacket packet)
        {
            var user = UserManagerRef.GetUser(packet.SessionID());
            if (user == null)
            {
                return;
            }

            try
            {
                var serializer = MessagePackSerializer.Get<DB.ResponseLogin>();
                var request = serializer.UnpackSingleObject(packet.GetData());

                if (request.Result == ERROR_CODE.NONE)
                {
                    UserManagerRef.유저_인증_완료(user);
                    DevLog.Write(string.Format("DB 인증 성공. ID:{0}", request.UserID), LOG_LEVEL.DEBUG);
                }
                else
                {
                    user.BasicState();
                    DevLog.Write(string.Format("DB 인증 실패. ID:{0}, Result:{1}", request.UserID, request.Result.ToString()), LOG_LEVEL.DEBUG);
                }

                var jsonData = new JsonPacketResponseLogin() { Result = request.Result };
                ServerNetworkRef.Send<JsonPacketResponseLogin>(packet.SessionID(), PACKET_ID.RESPONSE_LOGIN, jsonData);
            }
            catch (Exception ex)
            {
                DevLog.Write(ex.ToString(), LOG_LEVEL.DEBUG);
            }
        }

        public void RequestEnterLobby(SFNETPacket packet)
        {
            var user = UserManagerRef.GetUser(packet.SessionID());
            if (user == null)
            {
                return;
            }

            try
            {
                var request = JsonEnDecode.Decode<CSCommonLib.JsonPacketRequestEnterLobby>(packet.GetData());

                if (user.CurrentState != CONNECT_USER_STATE.ATUH_COMPLETE || user.UserID != request.UserID)
                {
                    return;
                }

                user.TryEnterLobby();

                RelayPacketPacketProcess.RelayPacket(request.LobbyID, packet);
            }
            catch (Exception ex)
            {
                DevLog.Write(ex.ToString(), LOG_LEVEL.DEBUG);
            }
        }

        public void InnerNotifyEnterLobby(SFNETPacket packet)
        {
            var notify = JsonEnDecode.Decode<CSCommonLib.JsonPacketInnerNotifyEnterLobby>(packet.GetData());

            var user = UserManagerRef.GetUser(notify.UserID);
            if (user == null)
            {
                var jsonData = new JsonPacketInnerRollBackEnterLobby() { LobbyID = notify.LobbyID, UserID = notify.UserID };
                var bodyData = CGSFNETCommon.JsonEnDecode.Encode<JsonPacketInnerRollBackEnterLobby>(jsonData);

                var InnerPacket = new CgsfNET64Lib.SFNETPacket();
                InnerPacket.SetData(-1, (ushort)PACKET_ID.INNER_ROLL_BACK_ENTER_LOBBY, bodyData);
                RelayPacketPacketProcess.RelayPacket(notify.LobbyID, InnerPacket);
                return;
            }

            if (notify.Result == ERROR_CODE.NONE)
            {
                user.SetLobby(notify.LobbyID);
            }
            else
            {
                user.SetLobby(0);
            }


            var response = new JsonPacketResponseEnterLobby() { Result = notify.Result, LobbyID = notify.LobbyID };
            ServerNetworkRef.Send<JsonPacketResponseEnterLobby>(user.SessionID, PACKET_ID.RESPONSE_ENTER_LOBBY, response);
        }


        public void InnerNotifyLeaveLobby(SFNETPacket packet)
        {
            var notify = JsonEnDecode.Decode<CSCommonLib.JsonPacketInnerNotifyLeaveLobby>(packet.GetData());

            var user = UserManagerRef.GetUser(notify.UserID);
            if (user == null)
            {
                return;
            }

            user.SetLobby(0);

            var response = new JsonPacketResponseLeaveLobby() { Result = notify.Result };
            ServerNetworkRef.Send<JsonPacketResponseLeaveLobby>(user.SessionID, PACKET_ID.RESPONSE_LEAVE_LOBBY, response);
        }
    }
}
