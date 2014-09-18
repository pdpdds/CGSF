using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace ChatServer1
{
    class Lobby
    {
        public int ID { get; private set; }

        // 로비에 들어올 수 있는 최대 유저 수
        int MaxUserCount = 0;

        // 로비에 들어온 유저 리스트
        LinkedList<ConnectUser> UserList = new LinkedList<ConnectUser>();



        public void Init(int id, int maxUserCount)
        {
            ID = id;
            MaxUserCount = maxUserCount;
        }

        public ERROR_CODE AddUser(ConnectUser user)
        {
            //if (MaxUserCount <= UserList.Count())
            //{
            //    return ERROR_CODE.ENTER_LOBBY_LOBBY_FULL;
            //}

            //UserList.AddLast(user);

            return ERROR_CODE.NONE;
        }

        public void RemoveUser(string userID)
        {
            //UserList.RemoveWhere(x => x.UserID == userID);
        }

        public void EnterLobbyComplete(string userID)
        {
            //var user = UserList.FirstOrDefault(x => x.UserID == userID);

            //if (user != null && string.IsNullOrEmpty(user.UserID) == false)
            //{
            //    user.ChangeStatusToEnterLobbyComplete();
            //}
        }

        public int CurrentUserCount()
        {
            return UserList.Count();
        }

        public void LobbyUserAbnormalNetwork(string userID)
        {
            //var user = UserList.FirstOrDefault(x => x.UserID == userID);
            //if (user != null && string.IsNullOrEmpty(user.UserID) == false)
            //{
            //    user.AbnormalNetwork();
            //}
        }

        public ERROR_CODE Chatting(byte[] sendChatData, ServerNetwork serverNetwork)
        {
            //UserList.ForEach(user =>
            //{
            //    if (user.EnableNetworkInLobby())
            //    {
            //        serverNetwork.SendData(user.SessionID, sendChatData, PACKETID.NTF_LOBBY_CHAT);
            //    }
            //}
            //    );
            return ERROR_CODE.NONE;
        }

        // 유저에게 서버에서 메시지를 보낸다.
        public void ServerNotification(string notifyMessage, ServerNetwork serverNetwork)
        {
            //var packet = new PKTNotification()
            //{
            //    Msg = notifyMessage
            //};

            //string jsonstring = Newtonsoft.Json.JsonConvert.SerializeObject(packet);
            //byte[] bodyData = Encoding.UTF8.GetBytes(jsonstring);
            //var sendData = PacketToBytes.Make(PACKETID.NTF_NOTIFICATION, bodyData);

            //UserList.ForEach(user =>
            //{
            //    if (user.EnableNetworkInLobby())
            //    {
            //        serverNetwork.SendData(user.SessionID, sendData, PACKETID.NTF_NOTIFICATION);
            //    }
            //}
            //    );
        }
    }
}
