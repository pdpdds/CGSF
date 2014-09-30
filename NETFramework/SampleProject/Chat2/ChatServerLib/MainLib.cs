using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

using CgsfNET64Lib;
using CGSFNETCommon;
using CSCommonLib;

namespace ChatServerLib
{
    public class MainLib
    {
        bool IsPacketDistributeThreadRunning;
        System.Threading.Thread PacketDistributeThread;

        bool IsStartServerNetwork = false;
        ServerNetwork ServerNet = new ServerNetwork();

        MainPacketProcessSystem MainPacketProcess;
        List<WorkPacketProcessSystem> WorkPacketProcessList = new List<WorkPacketProcessSystem>();
        DB.DBManager DBManager = new DB.DBManager();

        NetworkConfig NetConfig;
        ServerAppConfig AppConfig;

        List<int> LobbyIDToPacketProcessSystemIndexTable = new List<int>();


        public Tuple<bool, string> InitAndStartNetwork(ServerNetworkConfig netConfig, ServerAppConfig appConfig)
        {
            AppConfig = appConfig;
            NetConfig = new NetworkConfig()
            {
                IP = netConfig.IP,
                Port = netConfig.Port,
                EngineDllName = netConfig.EngineDllName,
                MaxAcceptCount = netConfig.MaxAcceptCount,
                ThreadCount = netConfig.ThreadCount,
                ProtocolOption = 0,
                ProtocolID = 0,
                MaxBufferSize = netConfig.MaxBufferSize,
                MaxPacketSize = netConfig.MaxPacketSize,
            };

            var result = ServerNet.Init(NetConfig, null, null);
            if (result != NET_ERROR_CODE_N.SUCCESS)
            {
                return new Tuple<bool, string>(false,result.ToString());
            }

            if (ServerNet.Start(NetConfig.ProtocolID) == false)
            {
                return new Tuple<bool, string>(false, NET_ERROR_CODE_N.NETWORK_START_FAIL.ToString());
            }
            else
            {
                IsStartServerNetwork = true;
            }           

            return new Tuple<bool, string>(true, NET_ERROR_CODE_N.SUCCESS.ToString());
        }

        public ERROR_CODE CreateSystem(ServerAppConfig appConfig)
        {
            if ((appConfig.MaxLobbyCount % appConfig.ProcessThreadCount) != 0)
            {
                return ERROR_CODE.INVALID_LOBBY_COUNT_PER_WORK_PACKET_PROCESS;
            }


            var lobbyCountPerWorkPacketProcess = appConfig.MaxLobbyCount / appConfig.ProcessThreadCount;

            var result = DBManager.CreateAndStart(1, DBResponseFunc);
            if (result != ERROR_CODE.NONE)
            {
                return result;
            }

            CreateAndStartPacketSysytem(appConfig, lobbyCountPerWorkPacketProcess);
            
            return ERROR_CODE.NONE;
        }

        public void Stop()
        {
            if (IsStartServerNetwork)
            {
                ServerNet.Stop();

                IsPacketDistributeThreadRunning = false;
                PacketDistributeThread.Join();

                DBManager.Destory();
            }
        }

        void CreateAndStartPacketSysytem(ServerAppConfig appConfig, int lobbyCountPerWorkPacketProcess)
        {
            MainPacketProcess = new MainPacketProcessSystem();
            MainPacketProcess.Init(appConfig, ServerNet, DBManager);


            SettingLobbyIDToPacketProcessIndexTable(appConfig.MaxLobbyCount, appConfig.ProcessThreadCount);


            for (int i = 1; i <= appConfig.ProcessThreadCount; ++i)
            {
                var process = new WorkPacketProcessSystem();
                process.Init(i, lobbyCountPerWorkPacketProcess, appConfig, ServerNet, DBManager);

                WorkPacketProcessList.Add(process);
            }


            RelayPacketPacketProcess.SetFunction(RelayPacketProcess);

            IsPacketDistributeThreadRunning = true;
            PacketDistributeThread = new System.Threading.Thread(this.DistributeProcket);
            PacketDistributeThread.Start();
        }              

        void SettingLobbyIDToPacketProcessIndexTable(int maxLobbyCount, int lobbyCountPerThread)
        {
            LobbyIDToPacketProcessSystemIndexTable.Add(0);

            int index = 1;

            for (int i = 1; i <= maxLobbyCount; ++i)
            {
                LobbyIDToPacketProcessSystemIndexTable.Add(index);

                if ((i % lobbyCountPerThread) == 0)
                {
                    ++index;
                }
            }
        }

        int GetPacketProcessIndex(int lobbyID)
        {
            if (lobbyID <= 0 || lobbyID > AppConfig.MaxLobbyCount)
            {
                return 0;
            }

            return LobbyIDToPacketProcessSystemIndexTable[lobbyID]; ;
        }

        void DistributeProcket()
        {
            while (IsPacketDistributeThreadRunning)
            {
                try
                {
                    var packet = ServerNet.GetPacket();
                    if (packet == null)
                    {
                        System.Threading.Thread.Sleep(1);
                        continue;
                    }
                        
                    var sessionID = packet.SessionID();
                    var packetProcessIndex = 0;
                        
                    if (packet.GetPacketType() == CgsfNET64Lib.SFPACKET_TYPE.DATA)
                    {
                        var user = MainPacketProcess.GetConnectUser(sessionID);
                        if (user != null)
                        {
                            packetProcessIndex = GetPacketProcessIndex(user.LobbyID);
                        }
                    }
                    else
                    {
                        if (packet.GetPacketType() == CgsfNET64Lib.SFPACKET_TYPE.CONNECT)
                        {
                            packet.SetData(sessionID, (ushort)PACKET_ID.SYSTEM_CLIENT_CONNECT, null);
                        }
                        else if (packet.GetPacketType() == CgsfNET64Lib.SFPACKET_TYPE.DISCONNECT)
                        {
                            packet.SetData(sessionID, (ushort)PACKET_ID.SYSTEM_CLIENT_DISCONNECTD, null);
                        }
                    }

                    if (packetProcessIndex == 0)
                    {
                        MainPacketProcess.ProcessPacket(packet);
                    }
                    else
                    {
                        WorkPacketProcessList[packetProcessIndex-1].InsertPacket(packet);
                    }
                }
                catch (Exception ex)
                {
                    DevLog.Write(ex.ToString(), LOG_LEVEL.ERROR);
                }
            }
        }

        void DBResponseFunc(DB.ResponseData resultData)
        {
            var packet = new SFNETPacket();
            packet.SetData(-1, (ushort)PACKET_ID.DB_RESPONSE_LOGIN, resultData.Datas);
            ServerNet.InnerPacket(packet);
        }

        public void RelayPacketProcess(short lobbyID, SFNETPacket packet)
        {
            var packetProcessIndex = GetPacketProcessIndex(lobbyID);

            if (packetProcessIndex == 0)
            {
                MainPacketProcess.ProcessPacket(packet);
            }
            else
            {
                WorkPacketProcessList[packetProcessIndex - 1].InsertPacket(packet);
            }
        }
    }

    public class ServerAppConfig
    {        
        public int MaxTotalUserCount;
        public int MaxLobbyCount;
        public int MaxLobbyUserCount;

        public int ProcessThreadCount;
        public int DBThreadCount;
    }

    public class ServerNetworkConfig
    {
        public string EngineDllName;
        public string IP;
        public int MaxAcceptCount;
        public int MaxBufferSize;
        public int MaxPacketSize;
        public ushort Port;
        public int ThreadCount;
    }
}
