using UnityEngine;
using System;
using System.IO;
using System.Collections.Generic;
using System.Text;
using System.Net;
using System.Net.Sockets;
using System.Threading;

namespace NETCODE
{
    /// <summary>
    /// 앞으로 기존의 동기식 NetManager를 대신하여 비동기식 NetManager_v2를 사용@
    /// </summary>
    public class NetManager_V2
    {
        public const int RECVDEFAULTSIZE = 25600;

        public delegate void DELEGATE_Connected(NetManager_V2 cs);
        public delegate void DELEGATE_Receive(jPacket packet);
        public delegate void DELEGATE_Disconnected(NetManager_V2 cs);
        public delegate void DELEGATE_AppExist();


        public DELEGATE_Connected delegate_Connected = null;
        public DELEGATE_Receive delegate_Receive = null;
        public DELEGATE_Disconnected delegate_Disconnected = null;
        public DELEGATE_AppExist delegate_AppExist = null;

        private AsyncCallback AsyncCallback_Receive = null;
        private AsyncCallback AsyncCallback_Send = null;

        private Queue<byte[]> btRecvQueue = null;
        private Queue<byte[]> btSendQueue = null;

        private Socket Sock = null;

        private byte[] btTmpSend = null;
        private int iTmpSendCount = 0;
        private int iTmpSendCursor = 0;

        private byte[] btTmpRecv = null;
        private int iTmpRecvCount = 0;

        private bool bSending = false;


        private string sIP = "";
        private int iPort = 25251;


        public NetManager_V2(string ip, int port)
        {
            init();
            sIP = ip;
            iPort = port;
        }
        void init()
        {
            btTmpRecv = new byte[RECVDEFAULTSIZE];
            AsyncCallback_Receive = new AsyncCallback(this.ReceiveCallback);
            AsyncCallback_Send = new AsyncCallback(this.SendCallback);
            delegate_Receive = ReadLoginPacketProc;
            btRecvQueue = new Queue<byte[]>();
            btSendQueue = new Queue<byte[]>();
        }
        public bool IsConnected()
        {
            if (Sock == null)
                return false;
            return Sock.Connected;
        }
        public void Send(byte[] data)
        {
            btSendQueue.Enqueue(data);
            sending();
        }
        private int ResetRecvCallBack(IAsyncResult ar)
        {
            return Sock.EndReceive(ar);
        }
        private int ResetSendCallBack(IAsyncResult ar)
        {
            return Sock.EndReceive(ar);
        }
        private void ReceiveCallback(IAsyncResult ar)//패킷을 받았을 때 @
        {//https://msdn.microsoft.com/ko-KR/library/windows/apps/system.servicemodel.channels.iinputchannel.endreceive(v=vs.85).aspx
            int readsize = ResetRecvCallBack(ar);
            if (readsize == 0)
            {
                Disconnect();
                return;
            }

            int iStartIndex = 0;
            iTmpRecvCount += readsize;//읽은길이 + 이번에 읽은 길이@

            while (11 <= iTmpRecvCount - iStartIndex) //전체 읽은길이 + 읽은 위치@
            {
                Int16 iProtocol = BitConverter.ToInt16(btTmpRecv, 0);
                Int32 iOption = BitConverter.ToInt32(btTmpRecv, 2);
                Int32 iCRC = BitConverter.ToInt32(btTmpRecv, 6);
                Int16 iDataSize = BitConverter.ToInt16(btTmpRecv, 10);

                int iTotalSize = jPacket.HEADERSIZE + iDataSize;

                if (iTmpRecvCount - iStartIndex >= iTotalSize)
                {
                    byte[] btData = new byte[iTotalSize];
                    Buffer.BlockCopy(btTmpRecv, iStartIndex, btData, 0, iTotalSize);

                    lock (this)
                    {
                        btRecvQueue.Enqueue(btData);
                    }

                    iStartIndex += iTotalSize;
                }
                else
                {
                    break;
                }
            }

            iTmpRecvCount -= iStartIndex;//읽은만큼 버퍼를 비우고 땡긴다@

            if (0 < iStartIndex)
            {
                Buffer.BlockCopy(btTmpRecv, iStartIndex, btTmpRecv, 0, iTmpRecvCount);
            }

            if (RECVDEFAULTSIZE > iTmpRecvCount)//버퍼크기 > 읽는 위치@
            {
                if (CheckExtraReceive() == false)//다시 beginreceive(). 이렇게 처리를 해줘야 한다 카더라@
                    Disconnect();
            }
            else
            {
                Disconnect();
            }
        }
        private void SendCallback(IAsyncResult ar)
        {
            lock (this)
            {
                int sendsize = Sock.EndSend(ar);

                if (sendsize < iTmpSendCount - iTmpSendCursor)//보낸크기 < (전체크기 - 보내기 완료한 크기) == 덜보냇다?
                {
                    iTmpSendCursor += sendsize;
                    Sock.BeginSend(btTmpSend, iTmpSendCursor, iTmpSendCount, (SocketFlags)0, AsyncCallback_Send, this);
                    return;
                }

                if (btSendQueue.Count == 0)//보낼게 없을 때 초기화@
                {
                    iTmpSendCount = 0;
                    iTmpSendCursor = 0;
                    btTmpSend = null;
                    bSending = false;
                    return;
                }

                byte[] data = btSendQueue.Dequeue();
                if (data == null)//새로 보낼 때 초기화@
                {
                    iTmpSendCount = 0;
                    iTmpSendCursor = 0;
                    btTmpSend = null;
                    bSending = false;
                    return;
                }

                iTmpSendCount = data.Length;//이번 패킷의 전체 길이@
                iTmpSendCursor = 0;//패킷 탐색 커서@
                btTmpSend = data;//보낼 패킷@

                try
                {
                    Sock.BeginSend(btTmpSend, iTmpSendCursor, iTmpSendCount, (SocketFlags)0, AsyncCallback_Send, this);//일딴 보낸다@
                }
                catch (Exception ex)
                {
                    Cloger.WR(ex.ToString());
                    bSending = false;
                }
            }
        }
        public bool Connect()
        {
            if(Sock != null)
            {
                Disconnect();
            }
            Sock = new Socket(AddressFamily.InterNetwork, SocketType.Stream, ProtocolType.Tcp);
            IPAddress host = IPAddress.Parse(sIP);
            IPEndPoint ipep = new IPEndPoint(host, iPort);
            try
            {
                Sock.Connect(ipep);
                if (Sock.Connected)
                {
                    OnConnected();
                }
            }
            catch (SocketException se)
            {
                Cloger.WR(se.ToString());
                return false;
            }
            return true;
        }
        public void Disconnect()
        {
            lock (this)
            {
                if (IsConnected())
                {
                    Sock.Shutdown(SocketShutdown.Both);
                    Sock.Disconnect(true);
                    Sock.Close();
                    Sock = null;
                }
            }
            if (Sock != null)
            {
                OnDisconnect();
            }
        }
        public void Delegate_Matching_Receive()
        {
            while (true)
            {
                if (btRecvQueue.Count <= 0)
                    break;
                byte[] data = null;
                lock (this)
                {
                    data = btRecvQueue.Dequeue();
                }
                if (data == null)
                    break;
                OnReceive(data);
            }
        }
        private void sending()
        {
            lock (this)
            {
                if (true == bSending)
                {
                    return;
                }

                if (null == btTmpSend)
                {
                    if (btSendQueue.Count <= 0)
                    {
                        return;
                    }

                    byte[] data = btSendQueue.Dequeue();
                    if (null == data)
                    {
                        return;
                    }
                    btTmpSend = data;
                    iTmpSendCount = data.Length;
                    iTmpSendCursor = 0;
                }

                try
                {
                    bSending = true;
                    Sock.BeginSend(btTmpSend, iTmpSendCursor, iTmpSendCount, (SocketFlags)0, AsyncCallback_Send, this);
                }
                catch (Exception ex)
                {
                    Cloger.WR(ex.ToString());
                    Disconnect();
                    bSending = false;
                }
            }
        }

        public void OnConnected()
        {
            Sock.ReceiveBufferSize = RECVDEFAULTSIZE;

            Sock.BeginReceive(btTmpRecv, iTmpRecvCount, RECVDEFAULTSIZE - iTmpRecvCount, (SocketFlags)0, AsyncCallback_Receive, this);
            if (null != delegate_Connected)
            {
                delegate_Connected(this);
            }
        }
        public void OnDisconnect()
        {
            if (null != delegate_Disconnected)
            {
                delegate_Disconnected(this);
            }
        }
        private bool CheckExtraReceive()
        {
            try
            {
                Sock.BeginReceive(btTmpRecv, iTmpRecvCount, RECVDEFAULTSIZE - iTmpRecvCount, (SocketFlags)0, AsyncCallback_Receive, this);

                return true;
            }
            catch (Exception ex)
            {
                Cloger.WR(ex.ToString());
                return false;
            }
        }
        public void OnReceive(byte[] btData)
        {
            jPacket packet = new jPacket();
            packet.Protocol = BitConverter.ToUInt16(btData, 0);//0~1
            packet.PacketOption = BitConverter.ToUInt32(btData, 2);//2~5
            packet.DataCRC = BitConverter.ToUInt32(btData, 6);//6~9
            packet.DataSize = BitConverter.ToUInt16(btData, 10);//10~11
            packet.JsonFormatData = new byte[packet.DataSize];
            Buffer.BlockCopy(btData, 12, packet.JsonFormatData, 0, packet.DataSize);
            if (delegate_Receive != null)
                delegate_Receive(packet);
            else
            {
                Cloger.WR("delegate_Receive == null");
            }
        }
        public void Runing()
        {
            Delegate_Matching_Receive();
        }
        public void ReadLoginPacketProc(jPacket packet)
        {
            switch (packet.Protocol)
            {
                case CommonProtocol.REQ_ECHO:
                    //CommonData.ECHO echo = jPacket.MakeObject<CommonData.ECHO>(packet.JsonFormatData);
                    break;
            }

        }
    }

}