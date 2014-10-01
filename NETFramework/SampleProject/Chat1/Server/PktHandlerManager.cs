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
    /// 핸들러 관리.
    /// 핸들러 등록, 패킷 처리
    /// </summary>
    class PktHandlerManager
    {
        Dictionary<ushort, Action<SFNETPacket, ConnectUser>> PacketHandlerMap = new Dictionary<ushort, Action<SFNETPacket, ConnectUser>>();

        ServerNetwork ServerNetworkRef;
        PktCommonHandler CommonPacketHandler;
        PktLobbyHandler LobbyPacketHandler;

        ConnectUserManager UserManager;
        LobbyManager LobbyMgr;



        public void Create(ServerNetwork serverNetwork, int maxLobbyCount, int maxLobbyUserCount)
        {
            ServerNetworkRef = serverNetwork;

            var totalUserCount = maxLobbyCount * maxLobbyUserCount;
            
            UserManager = new ConnectUserManager();
            UserManager.Init(totalUserCount);
            
            LobbyMgr = new LobbyManager();
            LobbyMgr.CreateLobby(ServerNetworkRef, maxLobbyCount, maxLobbyUserCount);


            RegistPacketHandler();
        }

        void RegistPacketHandler()
        {
            CommonPacketHandler = new PktCommonHandler();
            CommonPacketHandler.Init(ServerNetworkRef, UserManager, LobbyMgr);

            LobbyPacketHandler = new PktLobbyHandler();
            LobbyPacketHandler.Init(ServerNetworkRef, UserManager, LobbyMgr);
            
            PacketHandlerMap.Add((ushort)PACKET_ID.REQUEST_LOGIN, CommonPacketHandler.RequestLogin);
            PacketHandlerMap.Add((ushort)PACKET_ID.REQUEST_ENTER_LOBBY, LobbyPacketHandler.RequestEnterLobby);
            PacketHandlerMap.Add((ushort)PACKET_ID.REQUEST_LEAVE_LOBBY, LobbyPacketHandler.RequestLeaveLobby);
            PacketHandlerMap.Add((ushort)PACKET_ID.REQUEST_CHAT, LobbyPacketHandler.RequestChat);
        }

        public void Process(SFNETPacket packet)
        {
            var packetID = packet.PacketID();

            if (PacketHandlerMap.ContainsKey(packetID))
            {
                var user = UserManager.GetUser(packet.SessionID());

                PacketHandlerMap[packetID](packet, user);
            }
            else
            {
                DevLog.Write(string.Format("[PacketProcess] Invalid PacketID:{0}", packetID), LOG_LEVEL.ERROR);
            }
        }

        public void ClientDisConnect(int sessionID)
        {
            var user = UserManager.GetUser(sessionID);
            if (user != null && user.LobbyID > 0)
            {
                LobbyMgr.LeaveLobby(user.LobbyID, user.ID);
                InnerMessageQueue.CurrentLobbyUserCount(user.LobbyID, LobbyMgr.LobbyCurrentUserCount(user.LobbyID));
            }

            UserManager.RemoveUser(sessionID);
            
        }

    }
}
