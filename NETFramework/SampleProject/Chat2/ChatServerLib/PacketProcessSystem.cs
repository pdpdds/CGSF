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
    class PacketProcessSystem
    {
        int Index = -1;

        ServerNetwork ServerNetworkRef;

        bool IsThreadRunning = false;
        System.Threading.Thread ProcessThread = null;
        ConcurrentQueue<SFNETPacket> PacketQueue = new ConcurrentQueue<SFNETPacket>();
                
        Dictionary<ushort, Action<SFNETPacket>> PacketHandlerMap = new Dictionary<ushort, Action<SFNETPacket>>();
        PacketHandler.Common CommonHandler;

        ConnectUserManager UserManagerInst;
        LobbyManager LobbyManagerInst;


        public void Init(int index, ServerAppConfig appConfig, ServerNetwork serverNetwork)
        {
            Index = index;
            ServerNetworkRef = serverNetwork;

            // Index가 0 이면 Common 패킷 처리기를 뜻한다
            if (Index == 0)
            {
                UserManagerInst = new ConnectUserManager();
                UserManagerInst.Init(appConfig.MaxTotalUserCount);
            }
            else
            {
                LobbyManagerInst = new LobbyManager();
                LobbyManagerInst.Init(index, appConfig, serverNetwork);
            }


            RegistPacketHandler();

            IsThreadRunning = true;
            ProcessThread = new System.Threading.Thread(this.Process);
            ProcessThread.Start();
        }

        public void Stop()
        {
            IsThreadRunning = false;
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
            CommonHandler.Init(ServerNetworkRef, UserManagerInst, LobbyManagerInst);

            PacketHandlerMap.Add((ushort)CSCommonLib.PACKET_ID.SYSTEM_CLIENT_CONNECT, CommonHandler.SystemClientConnect);
            PacketHandlerMap.Add((ushort)CSCommonLib.PACKET_ID.SYSTEM_CLIENT_DISCONNECTD, CommonHandler.SystemClientDisConnected);



        }

        void Process()
        {
            SFNETPacket packet = null;

            while (IsThreadRunning)
            {
                try
                {
                    if (PacketQueue.TryDequeue(out packet) == false)
                    {
                        System.Threading.Thread.Sleep(1);
                        continue;
                    }

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
}
