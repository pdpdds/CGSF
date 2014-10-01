using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace ChatServer1
{
    /// <summary>
    /// 내부 통신용 메시지 큐
    /// </summary>
    class InnerMessageQueue
    {
        static System.Collections.Concurrent.ConcurrentQueue<InnerMsg> msgQueue = new System.Collections.Concurrent.ConcurrentQueue<InnerMsg>();

        
        static public bool GetMsg(out InnerMsg msg)
        {
            return msgQueue.TryDequeue(out msg);
        }

        public static void ServerStart(int ServerID, int Port)
        {
            var msg = new InnerMsg() { Type = InnerMsgType.SERVER_START };
            msg.Value1 = string.Format("{0}_{1}", ServerID, Port);

            msgQueue.Enqueue(msg);
        }

        public static void CreateComponent()
        {
            var msg = new InnerMsg() { Type = InnerMsgType.CREATE_COMPONENT };
            msgQueue.Enqueue(msg);
        }

        public static void CurrentLobbyUserCount(short lobbyID, int count)
        {
            var msg = new InnerMsg() { Type = InnerMsgType.CURRENT_LOBBY_USER_COUNT };
            msg.Value1 = string.Format("{0}_{1}", lobbyID, count);
            msgQueue.Enqueue(msg);
        }

        
    }

    public enum InnerMsgType
    {
        SERVER_START = 0,
        CREATE_COMPONENT,
        CURRENT_CONNECT_COUNT,
        
        CURRENT_LOBBY_USER_COUNT,
        
        END
    }

    public class InnerMsg
    {
        public InnerMsgType Type;
        public string SessionID;
        public string Value1;
        public string Value2;
    }
}
