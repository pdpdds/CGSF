using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

using CSCommonLib;

namespace ChatServerLib.Lobby
{
    class Lobby
    {
        public short ID { get; private set; }

        // 로비에 들어올 수 있는 최대 유저 수
        int MaxUserCount = 0;

        // 로비에 들어온 유저 리스트
        LinkedList<LobbyUser> UserList = new LinkedList<LobbyUser>();



        public void Init(short id, int maxUserCount)
        {
            ID = id;
            MaxUserCount = maxUserCount;
        }

        public ERROR_CODE AddUser(LobbyUser user)
        {
            if (MaxUserCount <= UserList.Count())
            {
                return ERROR_CODE.ENTER_LOBBY_LOBBY_FULL;
            }

            UserList.AddLast(user);

            return ERROR_CODE.NONE;
        }

        public bool RemoveUser(string userID)
        {
            if (UserList.Any(x => x.UserID == userID) == false)
            {
                return false;
            }

            UserList.RemoveWhere(x => x.UserID == userID);
            return true;
        }

        public int CurrentUserCount()
        {
            return UserList.Count();
        }

        public void Chatting(ServerNetwork serverNetwork, string userID, string chatMsg)
        {
            if (UserList.Any(x => x.UserID == userID) == false)
            {
                return;
            }

            var notify = new JsonPacketNoticeChat() { UserID = userID, Chat = chatMsg };

            UserList.ForEach(user =>
            {
                serverNetwork.Send<JsonPacketNoticeChat>(user.SessionID, PACKET_ID.NOTICE_CHAT, notify);
            }
            );
        }
    }
}
