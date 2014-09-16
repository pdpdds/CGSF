using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
//using System.Threading.Tasks;

using System.Net.Sockets;
using System.IO;
using System.Net;

namespace ChatClientNET
{
    public class ClientNetwork
    {
        public Socket Sock = null;   
        public string LatestErrorMsg;
        byte[] ReadBuffer = new byte[4096];



        //소켓연결        
        public bool Connect(string ip, int port)
        {
            try
            {
                IPAddress serverIP = IPAddress.Parse(ip);
                int serverPort = port;

                Sock = new Socket(AddressFamily.InterNetwork, SocketType.Stream, ProtocolType.Tcp);
                Sock.Connect(new IPEndPoint(serverIP, serverPort));

                if (Sock == null || Sock.Connected == false)
                {
                    return false;
                }

                return true;
            }
            catch (Exception ex)
            {
                LatestErrorMsg = ex.Message;
                return false;
            }
        }

        public ArraySegment<byte> Receive()
        {
            try
            {
                var nRecv = Sock.Receive(ReadBuffer, 0, ReadBuffer.Length, SocketFlags.None);

                if (nRecv == 0)
                {
                    return new ArraySegment<byte>();
                }

                return new ArraySegment<byte>(ReadBuffer, 0, nRecv);
            }
            catch (SocketException se)
            {
                LatestErrorMsg = se.Message;
            }

            return new ArraySegment<byte>();
        }

        //스트림에 쓰기
        public void Send(byte[] sendData)
        {
            try
            {
                if (Sock != null && Sock.Connected) //연결상태 유무 확인
                {
                    Sock.Send(sendData, 0, sendData.Length, SocketFlags.None);
                }
                else
                {
                    LatestErrorMsg = "먼저 채팅서버에 접속하세요!";
                }
            }
            catch (SocketException se)
            {
                LatestErrorMsg = se.Message;
            }
        }

        //소켓과 스트림 닫기
        public void Close()
        {
            if (Sock != null && Sock.Connected)
            {
                Sock.Shutdown(SocketShutdown.Both);
                Sock.Close();
            }
        }

        public bool IsConnected() { return (Sock != null && Sock.Connected) ? true : false; }
    }
}
