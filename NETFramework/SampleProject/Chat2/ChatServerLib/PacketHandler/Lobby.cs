using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

using CgsfNET64Lib;
using CGSFNETCommon;
using CSCommonLib;

namespace ChatServerLib.PacketHandler
{
    partial class Lobby : Base
    {
        ChatServerLib.Lobby.LobbyManager LobbyManagerRef;


        public void Init(ServerNetwork serverNetwork, DB.DBManager dbManager, ChatServerLib.Lobby.LobbyManager lobbyManager)
        {
            ServerNetworkRef = serverNetwork;
            DBManagerRef = dbManager;
            LobbyManagerRef = lobbyManager;
        }


        public void RequestEnterLobby(SFNETPacket packet)
        {
            try
            {
                var request = JsonEnDecode.Decode<JsonPacketRequestEnterLobby>(packet.GetData());

                // 로비에 추가
                var error = LobbyManagerRef.EnterLobby(request.LobbyID, packet.SessionID(), request.UserID);
                
                if (error == ERROR_CODE.NONE)
                {
                    InnerMessageQueue.CurrentLobbyUserCount(request.LobbyID, LobbyManagerRef.LobbyCurrentUserCount(request.LobbyID));
                }

                ResponseEnterLobby(error, request.LobbyID, request.UserID);
            }
            catch
            {
            }
        }


        public void InnerRoolBackEnterLobby(SFNETPacket packet)
        {
            try
            {
                var request = JsonEnDecode.Decode<JsonPacketInnerRollBackEnterLobby>(packet.GetData());

                // 로비에 추가
                var error = LobbyManagerRef.LeaveLobby(request.LobbyID, request.UserID);
                
                if (error == ERROR_CODE.NONE)
                {
                    InnerMessageQueue.CurrentLobbyUserCount(request.LobbyID, LobbyManagerRef.LobbyCurrentUserCount(request.LobbyID));
                }
            }
            catch
            {
            }
        }


        public void RequestLeaveLobby(SFNETPacket packetData)
        {
            try
            {
                var request = JsonEnDecode.Decode<JsonPacketRequestLeaveLobby>(packetData.GetData());

                var result = LobbyManagerRef.LeaveLobby(request.LobbyID, request.UserID);

                if (packetData.SessionID() >= 0)
                {
                    ResponseLeaveLobby(result, request.LobbyID, request.UserID);
                }

                if (result == ERROR_CODE.NONE)
                {
                    InnerMessageQueue.CurrentLobbyUserCount(request.LobbyID, LobbyManagerRef.LobbyCurrentUserCount(request.LobbyID));
                }
            }
            catch
            {
            }
        }


        public void RequestChat(SFNETPacket packetData)
        {
            try
            {
                var request = JsonEnDecode.Decode<JsonPacketRequestChat>(packetData.GetData());
                LobbyManagerRef.LobbyChat(request.LobbyID, request.UserID, request.Chat);
            }
            catch
            {
            }
        }
    }
}
