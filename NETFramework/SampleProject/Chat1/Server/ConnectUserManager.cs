using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace ChatServer1
{
    /// <summary>
    /// 연결된 유저 관리
    /// </summary>
    class ConnectUserManager
    {
        int MaxUserCount = 0;

        // 세션을 key로 사용하는 유저 맵
        Dictionary<int, ConnectUser> UserSessionMap = new Dictionary<int, ConnectUser>();
        // 유저 ID를 key를 사용하는 유저 맵
        Dictionary<string, ConnectUser> UserIDMap = new Dictionary<string, ConnectUser>();
        

        public void Init(int maxUserCount)
        {
            MaxUserCount = maxUserCount;
        }

        public ERROR_CODE AddUser(int sessionID, string userID)
        {
            if (UserSessionMap.ContainsKey(sessionID))
            {
                return ERROR_CODE.ADD_USER_DUPLICATION_SESSION;
            }

            if (UserIDMap.ContainsKey(userID))
            {
                return ERROR_CODE.ADD_USER_DUPLICATION_ID;
            }

            var user = new ConnectUser();
            user.SetInfo(sessionID, userID);

            UserSessionMap.Add(sessionID, user);
            UserIDMap.Add(userID, user);

            return ERROR_CODE.NONE;
        }

        public void RemoveUser(int serial)
        {
            var user = GetUser(serial);

            if (user != null)
            {
                UserSessionMap.Remove(serial);
                UserIDMap.Remove(user.ID);
            }
        }

        public ConnectUser GetUser(int serial)
        {
            ConnectUser user = null;
            UserSessionMap.TryGetValue(serial, out user);
            return user;
        }
    }

    class ConnectUser
    {
        public int SessionID { get; private set; }
        public string ID { get; private set; }
        public short LobbyID { get; private set; }

        public void SetInfo(int sessionID, string id)
        {
            SessionID = sessionID;
            ID = id;
        }

        public void EnterLobby(short lobbyID)
        {
            LobbyID = lobbyID;
        }

        public void LeaveLobby()
        {
            LobbyID = 0;
        }
    }
}
