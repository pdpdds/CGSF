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

namespace ChatClientNET
{
    public partial class MainForm : Form
    {
        ClientSimpleTcp Network = new ClientSimpleTcp();
        PacketBufferManager PacketBuffer = new PacketBufferManager();

        Queue<JsonPacketData> RecvPacketQueue = new Queue<JsonPacketData>();
        Queue<byte[]> SendPacketQueue = new Queue<byte[]>();

        bool IsNetworkThreadRunning = false;
        System.Threading.Thread NetworkReadThread = null;
        System.Threading.Thread NetworkSendThread = null;
        
        
        System.Windows.Threading.DispatcherTimer dispatcherUITimer;

        const int PACKET_ID_DISCONNECTED = 1;
        const int PACKET_ID_ECHO = 1000;
        


        public MainForm()
        {
            InitializeComponent();
        }

        private void MainForm_Load(object sender, EventArgs e)
        {
            PacketBuffer.Init((8096 * 10), 12, 512);

            IsNetworkThreadRunning = true;
            NetworkReadThread = new System.Threading.Thread(this.NetworkReadProcess);
            NetworkReadThread.Start();
            NetworkSendThread = new System.Threading.Thread(this.NetworkSendProcess);
            NetworkSendThread.Start();

            dispatcherUITimer = new System.Windows.Threading.DispatcherTimer();
            dispatcherUITimer.Tick += new EventHandler(ReadPacketQueueProcess);
            dispatcherUITimer.Interval = new TimeSpan(0, 0, 0, 0, 100);
            dispatcherUITimer.Start();

            btnDisconnect.Enabled = false;

            DevLog.Write("프로그램 시작 !!!", LOG_LEVEL.INFO);
        }

        private void MainForm_FormClosing(object sender, FormClosingEventArgs e)
        {
            Network.Close();

            IsNetworkThreadRunning = false;
            NetworkReadThread.Join();
            NetworkSendThread.Join();
        }

        // 접속하기
        private void btnConnect_Click(object sender, EventArgs e)
        {
            string address = textBoxIP.Text;

            if (checkBoxLocalHostIP.Checked)
            {
                address = "127.0.0.1";
            }

            int port = Convert.ToInt32(textBoxPort.Text);

            if (Network.Connect(address, port))
            {
                labelConnState.Text = string.Format("{0}. 서버에 접속 중", DateTime.Now);
                btnConnect.Enabled = false;
                btnDisconnect.Enabled = true;
            }
            else
            {
                labelConnState.Text = string.Format("{0}. 서버에 접속 실패", DateTime.Now);
            }
        }

        // 접속 종료
        private void button3_Click(object sender, EventArgs e)
        {
            SetDisconnectd();
            Network.Close();
        }
                
        // 에코
        private void button1_Click(object sender, EventArgs e)
        {
            var request = new JsonPacketNoticeEcho() { Msg = textBoxSendChat.Text };
            var bodyData = JsonEnDecode.Encode<JsonPacketNoticeEcho>(request);
            PostSendPacket((UInt16)PACKET_ID_ECHO, bodyData);
        }


        void SetDisconnectd()
        {
            if (btnConnect.Enabled == false)
            {
                btnConnect.Enabled = true;
                btnDisconnect.Enabled = false;
            }

            RecvPacketQueue.Clear();
            SendPacketQueue.Clear();

            labelConnState.Text = "서버 접속이 끊어짐";
        }

        void PostSendPacket(UInt16 packetID, byte[] bodyData)
        {
            List<byte> dataSource = new List<byte>();
            dataSource.AddRange(BitConverter.GetBytes(packetID));
            dataSource.AddRange(BitConverter.GetBytes((UInt32)0));
            dataSource.AddRange(BitConverter.GetBytes((UInt32)0));
            dataSource.AddRange(BitConverter.GetBytes((UInt16)bodyData.Length));
            dataSource.AddRange(bodyData);

            SendPacketQueue.Enqueue(dataSource.ToArray());
        }

        void NetworkReadProcess()
        {
            while (IsNetworkThreadRunning)
            {
                if (Network.IsConnected() == false)
                {
                    continue;
                }

                var recvData = Network.Receive();

                if (recvData.Count > 0)
                {
                    PacketBuffer.Write(recvData.Array, recvData.Offset, recvData.Count);

                    while (true)
                    {
                        var data = PacketBuffer.Read();
                        if (data.Count < 1)
                        {
                            break;
                        }

                        var packet = new JsonPacketData();
                        packet.PacketID = BitConverter.ToUInt16(data.Array, data.Offset);
                        packet.PacketOption = BitConverter.ToUInt32(data.Array, data.Offset+2);
                        packet.DataCRC = BitConverter.ToUInt32(data.Array, data.Offset+6);
                        packet.DataSize = BitConverter.ToUInt16(data.Array, data.Offset+10);
                        packet.JsonFormatData = new byte[packet.DataSize];
                        Buffer.BlockCopy(recvData.Array, (recvData.Offset + 12), packet.JsonFormatData, 0, (data.Count - 12));
                        
                        lock (((System.Collections.ICollection)RecvPacketQueue).SyncRoot)
                        {
                            RecvPacketQueue.Enqueue(packet);
                        }
                    }
                }
                else
                {
                    var packet = new JsonPacketData();
                    packet.PacketID = PACKET_ID_DISCONNECTED;
                    packet.DataSize = 0;
                    
                    lock (((System.Collections.ICollection)RecvPacketQueue).SyncRoot)
                    {
                        RecvPacketQueue.Enqueue(packet);
                    }
                }
            }
        }

        void NetworkSendProcess()
        {
            while (IsNetworkThreadRunning)
            {
                if (Network.IsConnected() == false)
                {
                    continue;
                }

                lock (((System.Collections.ICollection)RecvPacketQueue).SyncRoot)
                {
                    if (SendPacketQueue.Count > 0)
                    {
                        var packet = SendPacketQueue.Dequeue();
                        Network.Send(packet);
                    }
                }
            }
        }

        void ReadPacketQueueProcess(object sender, EventArgs e)
        {
            ProcessLog();

            try
            {
                JsonPacketData packet = null;
                lock (((System.Collections.ICollection)RecvPacketQueue).SyncRoot)
                {
                    if (RecvPacketQueue.Count() > 0)
                    {
                        packet = RecvPacketQueue.Dequeue();
                    }
                }

                if (packet != null)
                {
                    switch (packet.PacketID)
                    {
                        case PACKET_ID_DISCONNECTED:
                            SetDisconnectd();
                            break;

                        case PACKET_ID_ECHO:
                            {
                                var resData = JsonEnDecode.Decode<JsonPacketNoticeEcho>(packet.JsonFormatData);

                                textBoxSendChat.Text = "";
                                var msg = string.Format("[ECHO]: {0}", resData.Msg);
                                listBoxLog.Items.Add(msg);
                            }
                            break;

                        default:
                            break;
                    }
                }
            }
            catch (Exception ex)
            {
                MessageBox.Show(string.Format("ReadPacketQueueProcess. error:{0}", ex.Message));
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

                if (logWorkCount > 8)
                {
                    break;
                }
            }
        }



        class JsonPacketData
        {
            public UInt16 PacketID;
            public UInt32 PacketOption;
            public UInt32 DataCRC;
            public UInt16 DataSize;
            public byte[] JsonFormatData;
        }

        struct JsonPacketNoticeEcho
        {
            public string Msg;
        }

       
        

        
    }
}
