using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

using CgsfNET64Lib;
using CGSFNETCommon;

namespace GameServer
{
    class PktHandlerManager
    {
        Dictionary<ushort, Action<SFNETPacket>> PacketHandlerMap = new Dictionary<ushort, Action<SFNETPacket>>();

        ServerNetwork ServerNetworkRef;
        PktCommonHandler CommonPacketHandler;
        
        ConnectServerManager ServerManager;
        


        public void Create(ServerNetwork serverNetwork, int totalUserCount)
        {
            ServerNetworkRef = serverNetwork;

            ServerManager = new ConnectServerManager();
            
            RegistPacketHandler();
        }

        void RegistPacketHandler()
        {
            CommonPacketHandler = new PktCommonHandler();
            CommonPacketHandler.Init(ServerNetworkRef, ServerManager);


            PacketHandlerMap.Add((ushort)PACKET_ID.REQUEST_REGIST_SERVER, CommonPacketHandler.RequestRegistServer);
        }

        public void Process(SFNETPacket packet)
        {
            var packetID = packet.PacketID();

            if (PacketHandlerMap.ContainsKey(packetID))
            {
                PacketHandlerMap[packetID](packet);
            }
            else
            {
                DevLog.Write(string.Format("[PacketProcess] Invalid PacketID:{0}", packetID), LOG_LEVEL.ERROR);
            }
        }

        public void ServerDisConnect(int sessionID)
        {
            ServerManager.RemoveServer(sessionID);
        }

    }
}
