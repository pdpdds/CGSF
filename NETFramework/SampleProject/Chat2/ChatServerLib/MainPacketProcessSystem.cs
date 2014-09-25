using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Collections.Concurrent;

using CgsfNET64Lib;
using CGSFNETCommon;

namespace ChatServerLib
{
    class MainPacketProcessSystem
    {
        ServerNetwork ServerNetworkRef;

        ConcurrentQueue<SFNETPacket> PacketQueue = new ConcurrentQueue<SFNETPacket>();
                
        Dictionary<ushort, Action<SFNETPacket>> PacketHandlerMap = new Dictionary<ushort, Action<SFNETPacket>>();
        PacketHandler.Common CommonHandler;

        DB.DBManager DBManagerRef;
        ConnectUserManager UserManagerInst;
        

        public void Init(ServerAppConfig appConfig, ServerNetwork serverNetwork, DB.DBManager dbManager)
        {
            ServerNetworkRef = serverNetwork;
            DBManagerRef = dbManager;

            UserManagerInst = new ConnectUserManager();
            UserManagerInst.Init(appConfig.MaxTotalUserCount);
            

            RegistPacketHandler();
        }

        
        public void InsertPacket(SFNETPacket packet)
        {
            PacketQueue.Enqueue(packet);
        }

        public ConnectUser GetConnectUser(int sessionID)
        {
            return UserManagerInst.GetUser(sessionID);
        }


        void RegistPacketHandler()
        {
            CommonHandler = new PacketHandler.Common();
            CommonHandler.Init(ServerNetworkRef, DBManagerRef, UserManagerInst);

            PacketHandlerMap.Add((ushort)CSCommonLib.PACKET_ID.SYSTEM_CLIENT_CONNECT, CommonHandler.SystemClientConnect);
            PacketHandlerMap.Add((ushort)CSCommonLib.PACKET_ID.SYSTEM_CLIENT_DISCONNECTD, CommonHandler.SystemClientDisConnected);

            PacketHandlerMap.Add((ushort)CSCommonLib.PACKET_ID.REQUEST_LOGIN, CommonHandler.RequestLogin);
            PacketHandlerMap.Add((ushort)CSCommonLib.PACKET_ID.REQUEST_ENTER_LOBBY, CommonHandler.RequestEnterLobby);
            
            PacketHandlerMap.Add((ushort)CSCommonLib.PACKET_ID.DB_RESPONSE_LOGIN, CommonHandler.DBResponseLogin);
            PacketHandlerMap.Add((ushort)CSCommonLib.PACKET_ID.INNER_NOTIFY_ENTER_LOBBY, CommonHandler.InnerNotifyEnterLobby);
            PacketHandlerMap.Add((ushort)CSCommonLib.PACKET_ID.INNER_NOTIFY_LEAVE_LOBBY, CommonHandler.InnerNotifyLeaveLobby);
        }

        public void ProcessPacket(SFNETPacket packet)
        {
            try
            {
                var packetID = packet.PacketID();

                if (PacketHandlerMap.ContainsKey(packetID))
                {
                    PacketHandlerMap[packetID](packet);
                }
                else
                {
                    System.Diagnostics.Debug.WriteLine("세션 번호 {0}, PacketID {1}, 받은 데이터 크기: {2}", packet.SessionID(), packetID, packet.GetData().Length);
                }
            }
            catch (Exception ex)
            {
                DevLog.Write(ex.ToString(), LOG_LEVEL.ERROR);
            }
        }

        

    }
}
