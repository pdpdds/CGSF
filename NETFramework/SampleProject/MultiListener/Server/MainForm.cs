using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;

using CGSFNETCommon;

namespace GateWayServer
{
    public partial class MainForm : Form
    {
        System.Windows.Threading.DispatcherTimer workProcessTimer = new System.Windows.Threading.DispatcherTimer();

        bool IsStartServerNetwork = false;
        ServerNetwork ServerNet = new ServerNetwork();
        
        CgsfNET64Lib.NetworkConfig Config;
        List<CgsfNET64Lib.MultiListenNetworkInfo> MultiListenInfo = new List<CgsfNET64Lib.MultiListenNetworkInfo>();

        List<int> SessionList = new List<int>();

        PktHandlerManager HandelrMgr = new PktHandlerManager();

        

        public MainForm()
        {
            InitializeComponent();
        }

        private void MainForm_Load(object sender, EventArgs e)
        {
            workProcessTimer.Tick += new EventHandler(OnProcessTimedEvent);
            workProcessTimer.Interval = new TimeSpan(0, 0, 0, 0, 1);
            workProcessTimer.Start();

            Config = new CgsfNET64Lib.NetworkConfig()
            {
                IP = Properties.Settings.Default.IP,
                Port = 0,
                EngineDllName = Properties.Settings.Default.EngineDllName,
                MaxAcceptCount = Properties.Settings.Default.MaxAcceptCount,
                ThreadCount = Properties.Settings.Default.ThreadCount,
                ProtocolOption = 0,
                ProtocolID = 0,
                MaxBufferSize = Properties.Settings.Default.MaxBufferSize,
                MaxPacketSize = Properties.Settings.Default.MaxPacketSize,
            };

            MultiListenInfo.Add(new CgsfNET64Lib.MultiListenNetworkInfo() 
            {
                ListenID = 0,
                MaxBufferSize = Properties.Settings.Default.MaxBufferSize,
                MaxPacketSize = Properties.Settings.Default.MaxPacketSize,
                Port = 25252,
                ProtocolID = 1,
                ProtocolOption = 0 });
            MultiListenInfo.Add(new CgsfNET64Lib.MultiListenNetworkInfo()
            {
                ListenID = 0,
                MaxBufferSize = Properties.Settings.Default.MaxBufferSize,
                MaxPacketSize = Properties.Settings.Default.MaxPacketSize,
                Port = 25253,
                ProtocolID = 2,
                ProtocolOption = 0
            });


            try
            {
                var result = ServerNet.Init(Config, null, MultiListenInfo);
                if (result != CgsfNET64Lib.NET_ERROR_CODE_N.SUCCESS)
                {
                    DevLog.Write(string.Format("[Init] 네트워크 라이브러리 초기화 실패. {0}, {1}", result.ToString(), result), LOG_LEVEL.ERROR);
                    return;
                }
            }
            catch (Exception ex)
            {
                DevLog.Write(string.Format("[Init] 예외 발생. {0}", ex.ToString()), LOG_LEVEL.ERROR);
            }
            
            
            IsStartServerNetwork = true;
            if (ServerNet.Start(-1))
            {
                HandelrMgr.Create(ServerNet, Config.MaxAcceptCount);
                ConnectCountToGUI(0);

                DevLog.Write(string.Format("[Start] 네트워크 시작"), LOG_LEVEL.INFO);
            }
            else
            {
                IsStartServerNetwork = false;
                DevLog.Write(string.Format("[Start] 네트워크 시작 실패"), LOG_LEVEL.ERROR);
            }

            NetworkConfigInfoToGui();
        }

        private void MainForm_FormClosing(object sender, FormClosingEventArgs e)
        {
            if (IsStartServerNetwork)
            {
                DevLog.Write(string.Format("[Stop] 네트워크 종료"), LOG_LEVEL.INFO);
                ServerNet.Stop();
            }
        }

        // 게임서버에 연결하기
        private void button1_Click(object sender, EventArgs e)
        {
        }

        void NetworkConfigInfoToGui()
        {
            textBoxServerConfig.AppendText(string.Format("ThreadCount:{0}, EngineDllName:{1} {2}", Config.ThreadCount, Config.EngineDllName, Environment.NewLine));

            foreach(var listen in MultiListenInfo)
            {
                textBoxServerConfig.AppendText(string.Format("listenID:{0}, IP:{1}, Port:{2}, ProtocolID:{3}, MaxBufferSize:{4}, MaxPacketSize:{5} {6}",
                    listen.ListenID, Config.IP, listen.Port, listen.ProtocolID, listen.MaxBufferSize, listen.MaxPacketSize, Environment.NewLine));
            }
        }

        void ConnectCountToGUI(int count)
        {
            textBoxConnectCount.Text = string.Format("현재 접속 수: {0}",count.ToString());
        }
                       
        private void OnProcessTimedEvent(object sender, EventArgs e)
        {
            try
            {
                ProcessProcket();
                ProcessInnerMessage();
                ProcessLog();
            }
            catch (Exception ex)
            {
                DevLog.Write(string.Format("[OnProcessTimedEvent] Exception:{0}", ex.ToString()), LOG_LEVEL.ERROR);
            }
        }

        private void ProcessProcket()
        {
            var packet = ServerNet.GetPacket();
            if (packet == null)
            {
                return;
            }

            switch (packet.GetPacketType())
            {
                case CgsfNET64Lib.SFPACKET_TYPE.CONNECT:
                    if (packet.IsServerConnect())
                    {
                        var response = new JsonPacketRequestReqistServer() { ServerName = "GateWayServer" };
                        ServerNet.Send<JsonPacketRequestReqistServer>(packet.SessionID(), PACKET_ID.REQUEST_REGIST_SERVER, response);

                        DevLog.Write(string.Format("[OnConnect Server] SessionID:{0}, ConnectID:{1}", packet.SessionID(), packet.ServerIdentifier()), LOG_LEVEL.INFO);
                    }
                    else
                    {
                        SessionList.Add(packet.SessionID());
                        ConnectCountToGUI(SessionList.Count());
                        DevLog.Write(string.Format("[OnConnect Client] SessionID:{0}", packet.SessionID()), LOG_LEVEL.INFO);
                    }
                    break;
                case CgsfNET64Lib.SFPACKET_TYPE.DISCONNECT:
                    if (packet.IsServerConnect())
                    {
                        DevLog.Write(string.Format("[OnDisConnect Server] SessionID:{0}", packet.SessionID()), LOG_LEVEL.INFO);
                    }
                    else
                    {
                        SessionList.Remove(packet.SessionID());
                        HandelrMgr.ClientDisConnect(packet.SessionID());
                        ConnectCountToGUI(SessionList.Count());
                        DevLog.Write(string.Format("[OnDisConnect Client] SessionID:{0}", packet.SessionID()), LOG_LEVEL.INFO);
                    }
                    break;
                case CgsfNET64Lib.SFPACKET_TYPE.DATA:
                    HandelrMgr.Process(packet);
                    break;
            }
        }

        void ProcessInnerMessage()
        {
            //while (true)
            //{
            //    InnerMsg msg;

            //    if (InnerMessageQueue.GetMsg(out msg))
            //    {
            //        switch (msg.Type)
            //        {
                        
            //        }
            //    }
            //    else
            //    {
            //        break;
            //    }
            //}
        }

        private void ProcessLog()
        {
            // 너무 이 작업만 할 수 없으므로 일정 작업 이상을 하면 일단 패스한다.
            int logWorkCount = 0;

            while (true)
            {
                string msg;

                if (DevLog.GetLog(out msg))
                {
                    ++logWorkCount;

                    if (listBoxLog.Items.Count > 512)
                    {
                        listBoxLog.Items.Clear();
                    }

                    listBoxLog.Items.Add(msg);
                    listBoxLog.SelectedIndex = listBoxLog.Items.Count - 1;
                }
                else
                {
                    break;
                }

                if (logWorkCount > 32)
                {
                    break;
                }
            }
        }

        

        

        
    }
}
