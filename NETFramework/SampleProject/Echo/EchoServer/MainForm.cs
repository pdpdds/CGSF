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

namespace EchoServer
{
    public partial class MainForm : Form
    {
        System.Windows.Threading.DispatcherTimer workProcessTimer = new System.Windows.Threading.DispatcherTimer();

        /// <summary>
        /// 네트워크 동작 여부
        /// </summary>
        bool IsStartServerNetwork = false;

        CgsfNET64Lib.NetworkConfig Config;

        /// <summary>
        /// CGSF의 서버 네트워크 라이브러리
        /// </summary>
        ServerNetwork ServerNet = new ServerNetwork();

        /// <summary>
        /// 현재 연결된 세션 리스트
        /// </summary>
        List<int> SessionList = new List<int>();

        /// <summary>
        /// 에코 패킷 ID
        /// </summary>
        const UInt16 PACKET_ID_ECHO = 1000;
        

        public MainForm()
        {
            InitializeComponent();
        }

        private void MainForm_Load(object sender, EventArgs e)
        {
            // 반복 처리를 위한 스레드(UI 동기)
            workProcessTimer.Tick += new EventHandler(OnProcessTimedEvent);
            workProcessTimer.Interval = new TimeSpan(0, 0, 0, 0, 32);
            workProcessTimer.Start();

            Config = new CgsfNET64Lib.NetworkConfig()
            {
                IP = "127.0.0.1",
                Port = 25251,
                EngineDllName = "CGSFNet.dll",
                MaxAcceptCount = 1000,
                ThreadCount = 4,
                ProtocolOption = 0,
                ProtocolID = 0,
                MaxBufferSize = 16000,
                MaxPacketSize = 4012,
            };

            var result = ServerNet.Init(Config, null, null);
            if (result == CgsfNET64Lib.NET_ERROR_CODE_N.SUCCESS)
            {
                DevLog.Write(string.Format("[Init] IP:{0}, Port:{1}, EngineDllName:{2}", Config.IP, Config.Port, Config.EngineDllName), LOG_LEVEL.INFO);
                
            }
            else
            {
                DevLog.Write(string.Format("[Init] 네트워크 라이브러리 초기화 실패. {0}, {1}", result.ToString(), result), LOG_LEVEL.ERROR);
            }
        }

        private void MainForm_FormClosing(object sender, FormClosingEventArgs e)
        {
            if (IsStartServerNetwork)
            {
                DevLog.Write(string.Format("[Stop] 네트워크 종료"), LOG_LEVEL.INFO);
                ServerNet.Stop();
            }
        }

        private void button1_Click(object sender, EventArgs e)
        {
            IsStartServerNetwork = true;

            var result = ServerNet.Start(Config.ProtocolID);

            if (result)
            {
                DevLog.Write(string.Format("[Start] 네트워크 시작"), LOG_LEVEL.INFO);
            }
            else
            {
                DevLog.Write(string.Format("[Start] 네트워크 시작 실패"), LOG_LEVEL.ERROR);
            }
        }

        private void button2_Click(object sender, EventArgs e)
        {

        }
        
       
        private void OnProcessTimedEvent(object sender, EventArgs e)
        {
            try
            {
                ProcessProcket();
                ProcessLog();
            }
            catch (Exception ex)
            {
                DevLog.Write(string.Format("[OnProcessTimedEvent] Exception:{0}", ex.ToString()), LOG_LEVEL.ERROR);
            }
        }

        /// <summary>
        /// 패킷 처리
        /// </summary>
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
                    SessionList.Add(packet.SessionID());
                    DevLog.Write(string.Format("[OnConnect] Serial:{0}", packet.SessionID()), LOG_LEVEL.INFO);
                    break;
                case CgsfNET64Lib.SFPACKET_TYPE.DISCONNECT:
                    SessionList.Remove(packet.SessionID());
                    DevLog.Write(string.Format("[OnDisConnect] Serial:{0}", packet.SessionID()), LOG_LEVEL.INFO);
                    break;
                case CgsfNET64Lib.SFPACKET_TYPE.DATA:
                    switch (packet.PacketID())
                    {
                        case PACKET_ID_ECHO:
                            var resData = JsonEnDecode.Decode<JsonPacketNoticeEcho>(packet.GetData());
                            DevLog.Write(string.Format("[Chat] Serial:{0}, Msg:{1}", packet.SessionID(), resData.Msg), LOG_LEVEL.INFO);

                            var request = new JsonPacketNoticeEcho() { Msg = resData.Msg };
                            var bodyData = JsonEnDecode.Encode<JsonPacketNoticeEcho>(request);
                            ServerNet.SendPacket(packet.SessionID(), PACKET_ID_ECHO, bodyData);
                            break;
                        default:
                            DevLog.Write(string.Format("[ProcessProcket] Invalid PacketID:{0}", packet.PacketID()), LOG_LEVEL.ERROR);
                            break;
                    }
                    break;
            }
        }

        /// <summary>
        /// 개발 로그 UI에 출력
        /// </summary>
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


        /// <summary>
        /// 에코 패킷 데이터
        /// </summary>
        struct JsonPacketNoticeEcho
        {
            public string Msg;
        }

        
    }
}
