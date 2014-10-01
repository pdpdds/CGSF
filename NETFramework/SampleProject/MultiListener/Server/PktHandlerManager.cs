using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

using CgsfNET64Lib;
using CGSFNETCommon;

namespace GateWayServer
{
    class PktHandlerManager
    {
        Dictionary<ushort, Action<SFNETPacket, ConnectUser>> ClientPacketHandlerMap = new Dictionary<ushort, Action<SFNETPacket, ConnectUser>>();
        Dictionary<ushort, Action<SFNETPacket>> ServerPacketHandlerMap = new Dictionary<ushort, Action<SFNETPacket>>();

        ServerNetwork ServerNetworkRef;
        PktCommonHandler CommonPacketHandler;
       
        ConnectUserManager UserManager;
        


        public void Create(ServerNetwork serverNetwork, int totalUserCount)
        {
            ServerNetworkRef = serverNetwork;

            UserManager = new ConnectUserManager();
            UserManager.Init(totalUserCount);                     

            RegistPacketHandler();
        }

        void RegistPacketHandler()
        {
            CommonPacketHandler = new PktCommonHandler();
            CommonPacketHandler.Init(ServerNetworkRef, UserManager);
                        
            ClientPacketHandlerMap.Add((ushort)PACKET_ID.REQUEST_LOGIN, CommonPacketHandler.RequestLogin);

            ServerPacketHandlerMap.Add((ushort)PACKET_ID.RESPONSE_REGIST_SERVER, CommonPacketHandler.ResponseRegistServer);
        }

        public void Process(SFNETPacket packet)
        {
            var packetID = packet.PacketID();

            if (packetID > (ushort)PACKET_ID.CLIENT_RANGE_FIRST && packetID < (ushort)PACKET_ID.CLIENT_RANGE_LAST)
            {
                if (ClientPacketHandlerMap.ContainsKey(packetID))
                {
                    var user = UserManager.GetUser(packet.SessionID());
                    ClientPacketHandlerMap[packetID](packet, user);
                    return;
                }
            }

            if (packetID > (ushort)PACKET_ID.SERVER_RANGE_FIRST && packetID < (ushort)PACKET_ID.SERVER_RANGE_LAST)
            {
                if (ServerPacketHandlerMap.ContainsKey(packetID))
                {
                    ServerPacketHandlerMap[packetID](packet);
                    return;
                }
            }

            DevLog.Write(string.Format("[PacketProcess] Invalid PacketID:{0}", packetID), LOG_LEVEL.ERROR);
        }

        public void ClientDisConnect(int sessionID)
        {
            UserManager.RemoveUser(sessionID);
            
        }

    }
}
