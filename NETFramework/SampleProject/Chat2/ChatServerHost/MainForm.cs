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
using ChatServerLib;


namespace ChatServerHost
{
    public partial class MainForm : Form
    {
        System.Windows.Threading.DispatcherTimer workProcessTimer = new System.Windows.Threading.DispatcherTimer();

        ChatServerLib.MainLib ServerLib = new ChatServerLib.MainLib();


        public MainForm()
        {
            InitializeComponent();
        }

        private void Form1_Load(object sender, EventArgs e)
        {
            workProcessTimer.Tick += new EventHandler(OnProcessTimedEvent);
            workProcessTimer.Interval = new TimeSpan(0, 0, 0, 0, 1);
            workProcessTimer.Start();


            var netConfig = new ChatServerLib.ServerNetworkConfig()
            {
                IP = "127.0.0.1",
                Port = textBoxPort.Text.ToUInt16(),
                EngineDllName = "CGSFNet.dll",
                MaxAcceptCount = textBoxMaxUserCount.Text.ToInt32(),
                ThreadCount = textBoxPacketThreadCount.Text.ToInt32(),
                MaxBufferSize = textBoxMaxBufferSize.Text.ToInt32(),
                MaxPacketSize = textBoxMaxPacketSize.Text.ToInt32(),
            };

            var appConfig = new ChatServerLib.ServerAppConfig()
            {
                MaxTotalUserCount = textBoxMaxUserCount.Text.ToInt32(),
                MaxLobbyCount = textBoxLobbyCount.Text.ToInt32(),
                MaxLobbyUserCount = textBoxMaxUserPerLobby.Text.ToInt32(),
                ProcessThreadCount = textBoxPacketThreadCount.Text.ToInt32(),
                DBThreadCount = textBoxDBThreadCount.Text.ToInt32(),
            };

            if (((appConfig.ProcessThreadCount - 1) % appConfig.MaxLobbyCount) != 0)
            {
                ++appConfig.ProcessThreadCount;
            }
            

            var result = ServerLib.InitAndStartNetwork(netConfig, appConfig);
            if (result.Item1 == false)
            {
                DevLog.Write(string.Format("[Init] 네트워크 라이브러리 초기화 및 시작 실패. {0}", result.Item2.ToString()), LOG_LEVEL.ERROR);
                return;
            }

            DevLog.Write(string.Format("[Start] 네트워크 시작"), LOG_LEVEL.INFO);
        }

        private void Form1_FormClosing(object sender, FormClosingEventArgs e)
        {
            ServerLib.Stop();
        }


        private void OnProcessTimedEvent(object sender, EventArgs e)
        {
            try
            {
                ProcessInnerMessage();
                ProcessLog();
            }
            catch (Exception ex)
            {
                DevLog.Write(string.Format("[OnProcessTimedEvent] Exception:{0}", ex.ToString()), LOG_LEVEL.ERROR);
            }
        }
                
        void ProcessInnerMessage()
        {
            while (true)
            {
                InnerMsg msg;

                if (InnerMessageQueue.GetMsg(out msg))
                {
                    switch (msg.Type)
                    {
                        case InnerMsgType.CURRENT_CONNECT_COUNT:
                            {
                                textBoxCurrentUserCount.Text = msg.Value1;
                            }
                            break;

                        case InnerMsgType.CURRENT_LOBBY_USER_COUNT:
                            {
                                var tokens = msg.Value1.Split("_");

                                int iItemNum = listViewLobbyInfo.Items.Count;

                                for (int i = 0; i < iItemNum; ++i)
                                {
                                    if (tokens[0] == listViewLobbyInfo.Items[i].SubItems[0].Text)
                                    {
                                        listViewLobbyInfo.Items[i].SubItems[1].Text = tokens[1];
                                        listViewLobbyInfo.Refresh();
                                        return;
                                    }
                                }
                            }
                            break;

                    }
                }
                else
                {
                    break;
                }
            }
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
    } // End MainForm


    

}
