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
    class WorkPacketProcessSystem
    {
        int Index = -1;

        ServerNetwork ServerNetworkRef;

        bool IsThreadRunning = false;
        System.Threading.Thread ProcessThread = null;
        ConcurrentQueue<SFNETPacket> PacketQueue = new ConcurrentQueue<SFNETPacket>();
                
        Dictionary<ushort, Action<SFNETPacket>> PacketHandlerMap = new Dictionary<ushort, Action<SFNETPacket>>();
        PacketHandler.Lobby LobbyHandler;

        DB.DBManager DBManagerRef;
        Lobby.LobbyManager LobbyManagerInst;


        public void Init(int index, int lobbyCountPerWorkPacketProcess, ServerAppConfig appConfig, ServerNetwork serverNetwork, DB.DBManager dbManager)
        {
            Index = index;
            ServerNetworkRef = serverNetwork;
            DBManagerRef = dbManager;

            var firstLobbyID = ((Index - 1) * lobbyCountPerWorkPacketProcess) + 1;
            if (Index != 1)
            {
                firstLobbyID = Index * lobbyCountPerWorkPacketProcess;
            } 

            LobbyManagerInst = new Lobby.LobbyManager();
            LobbyManagerInst.Init(firstLobbyID, lobbyCountPerWorkPacketProcess, appConfig, serverNetwork);
            

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
               
        void RegistPacketHandler()
        {
            LobbyHandler = new PacketHandler.Lobby();
            LobbyHandler.Init(ServerNetworkRef, DBManagerRef, LobbyManagerInst);

            PacketHandlerMap.Add((ushort)CSCommonLib.PACKET_ID.REQUEST_LEAVE_LOBBY, LobbyHandler.RequestEnterLobby);


            PacketHandlerMap.Add((ushort)CSCommonLib.PACKET_ID.INNER_ROLL_BACK_ENTER_LOBBY, LobbyHandler.InnerRoolBackEnterLobby);
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
