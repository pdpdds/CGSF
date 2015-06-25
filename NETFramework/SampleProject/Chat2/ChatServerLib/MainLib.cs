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
    /// <summary>
    /// 채팅 서버 로직 메인 클래스
    /// </summary>
    public class MainLib
    {
        // 패킷 디스트리뷰트 스레드 동작 여부
        bool IsPacketDistributeThreadRunning;
        // 패킷 처리 스레드에 패킷을 할당하는 스레드
        System.Threading.Thread PacketDistributeThread;

        bool IsStartServerNetwork = false;
        ServerNetwork ServerNet = new ServerNetwork();

        // 메인 패킷 처리. 주
        MainPacketProcessSystem MainPacketProcess;
        // 로비 관련 패킷을 처리하는 스레드
        // 각각의 스레드에서 할당된 로비 객체를 다룬다.
        List<WorkPacketProcessSystem> WorkPacketProcessList = new List<WorkPacketProcessSystem>();
        
        DB.DBManager DBManager = new DB.DBManager();

        NetworkConfig NetConfig;
        ServerAppConfig AppConfig;

        // 로비 ID-WorkPacketProcessList의 인덱스를 맵핑
        // 예를들면 로비ID가 7번인 경우 WorkPacketProcessList의 몇번째 객체를 사용해야하는지 설정되어 있다.
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
            if (appConfig.ProcessThreadCount < 2)
            {
                return ERROR_CODE.LESS_PACKET_PROCESS_THREAD_COUNT;
            }

            var lobbyProcessThreadCount = appConfig.ProcessThreadCount - 1;
            if ((appConfig.MaxLobbyCount % lobbyProcessThreadCount) != 0)
            {
                return ERROR_CODE.INVALID_LOBBY_COUNT_PER_WORK_PACKET_PROCESS;
            }


            var lobbyCountPerWorkPacketProcess = appConfig.MaxLobbyCount / lobbyProcessThreadCount;

            var result = DBManager.CreateAndStart(1, DBResponseFunc);
            if (result != ERROR_CODE.NONE)
            {
                return result;
            }

            CreateAndStartPacketSysytem(appConfig, lobbyProcessThreadCount, lobbyCountPerWorkPacketProcess);
            
            return ERROR_CODE.NONE;
        }

        public void Stop()
        {
            if (IsStartServerNetwork)
            {
                ServerNet.Stop();

                IsPacketDistributeThreadRunning = false;

                if (PacketDistributeThread != null)
                    PacketDistributeThread.Join();

                DBManager.Destory();
            }
        }

        void CreateAndStartPacketSysytem(ServerAppConfig appConfig, int lobbyProcessThreadCount, int lobbyCountPerWorkPacketProcess)
        {
            MainPacketProcess = new MainPacketProcessSystem();
            MainPacketProcess.Init(appConfig, ServerNet, DBManager);


            SettingLobbyIDToPacketProcessIndexTable(appConfig.MaxLobbyCount, lobbyProcessThreadCount);


            for (int i = 0; i <= lobbyProcessThreadCount; ++i)
            {
                var process = new WorkPacketProcessSystem();
                process.Init(i+1, lobbyCountPerWorkPacketProcess, appConfig, ServerNet, DBManager);

                WorkPacketProcessList.Add(process);
            }


            RelayPacketPacketProcess.SetFunction(RelayPacketProcess);

            IsPacketDistributeThreadRunning = true;
            PacketDistributeThread = new System.Threading.Thread(this.DistributeProcket);
            PacketDistributeThread.Start();
        }              

        /// <summary>
        /// LobbyIDToPacketProcessSystemIndexTable을 설정한다.
        /// </summary>
        /// <param name="maxLobbyCount">최대 로비 수</param>
        /// <param name="lobbyCountPerThread">스레드 당 처리할 로비 수</param>
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

        /// <summary>
        /// 로비ID를 기준으로 패킷을 각각의 패킷처리 객체에 할당한다. 멀티스레드로 패킷을 처리하도록 해준다.
        /// </summary>
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

        // DB 처리 결과를 내부 패킷으로 넣는다. DB 처리 스레드에서 DB 처리 후 호출한다.
        void DBResponseFunc(DB.ResponseData resultData)
        {
            var packet = new SFNETPacket();
            packet.SetData(-1, (ushort)PACKET_ID.DB_RESPONSE_LOGIN, resultData.Datas);
            ServerNet.InnerPacket(packet);
        }

        // 로비 ID에 맞는 패킷처리 객체로 전달한다.
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

    // ChatServerHost에서 네트워크 라이브러리를 참조하지 않게 하기 위해서 네트워크 설정 클래스를 재정의
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
