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

        List<PacketProcessSystem> PacketProcessSystemList = new List<PacketProcessSystem>();
        DB.DBManager DBManager = new DB.DBManager();

        NetworkConfig NetConfig;
        ServerAppConfig AppConfig;

        List<int> LobbyIDToPacketProcessSystemIndexTable = new List<int>();


        public NET_ERROR_CODE_N InitAndStartNetwork(NetworkConfig config, ServerAppConfig appConfig)
        {
            AppConfig = appConfig;
            NetConfig = config;

            var result = ServerNet.Init(NetConfig);
            if (result != NET_ERROR_CODE_N.SUCCESS)
            {
                return result;
            }

            if (ServerNet.Start() == false)
            {
                return NET_ERROR_CODE_N.NETWORK_START_FAIL;
            }
            else
            {
                IsStartServerNetwork = true;
            }


            CreateAndStartPacketSysytem(appConfig);


            DBManager.CreateAndStart(1, DBResponseFunc); 

            return NET_ERROR_CODE_N.SUCCESS;
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

        void CreateAndStartPacketSysytem(ServerAppConfig appConfig)
        {
            SettingLobbyIDToPacketProcessSystemIndexTable(appConfig.MaxLobbyCount, appConfig.ProcessThreadCount);


            for (int i = 0; i < appConfig.ProcessThreadCount; ++i)
            {
                var process = new PacketProcessSystem();
                process.Init(i, appConfig, ServerNet);

                PacketProcessSystemList.Add(process);
            }


            IsPacketDistributeThreadRunning = true;
            PacketDistributeThread = new System.Threading.Thread(this.DistributeProcket);
            PacketDistributeThread.Start();
        }                

        void SettingLobbyIDToPacketProcessSystemIndexTable(int lobbyCount, int threadCount)
        {
            LobbyIDToPacketProcessSystemIndexTable.Add(0);

            int index = 1;

            for (int i = 1; i <= lobbyCount; ++i)
            {
                LobbyIDToPacketProcessSystemIndexTable.Add(index);

                if ((i % lobbyCount) == 0)
                {
                    ++index;
                }
            }
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
                        return;
                    }


                    var sessionID = packet.SessionID();
                    var packetProcessIndex = 0;

                    if (packet.GetPacketType() == CgsfNET64Lib.SFPACKET_TYPE.CONNECT)
                    {
                        packet.SetData((ushort)PACKET_ID.SYSTEM_CLIENT_CONNECT, null);
                    }
                    else if (packet.GetPacketType() == CgsfNET64Lib.SFPACKET_TYPE.DISCONNECT)
                    {
                        packet.SetData((ushort)PACKET_ID.SYSTEM_CLIENT_DISCONNECTD, null);
                    }
                    else if (packet.GetPacketType() == CgsfNET64Lib.SFPACKET_TYPE.DATA)
                    {
                        var user = PacketProcessSystemList[0].GetConnectUser(sessionID);
                        if (user != null)
                        {
                            packetProcessIndex = LobbyIDToPacketProcessSystemIndexTable[user.LobbyID];

                            if (packetProcessIndex == 0 && packet.PacketID() == (ushort)PACKET_ID.REQUEST_ENTER_LOBBY)
                            {
                                var request = JsonEnDecode.Decode<JsonPacketRequestEnterLobby>(packet.GetData());
                                if (request.LobbyID >= 0 && request.LobbyID <= AppConfig.MaxLobbyCount)
                                {
                                    packetProcessIndex = LobbyIDToPacketProcessSystemIndexTable[request.LobbyID];
                                }
                            }
                        }
                    }

                    PacketProcessSystemList[packetProcessIndex].InsertPacket(packet);
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
            packet.SetData((ushort)PACKET_ID.DB_RESPONSE_LOGIN, resultData.Datas);
            ServerNet.InnerPacket(packet);
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
}
