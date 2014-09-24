using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

using CSCommonLib;

namespace ChatServerLib
{
    class ConnectUserManager
    {
        int MaxUserCount = 0;

        Int64 UserSequenceNumber = 0;

        Dictionary<int, ConnectUser> UserSessionMap = new Dictionary<int, ConnectUser>();
        Dictionary<string, ConnectUser> UserIDMap = new Dictionary<string, ConnectUser>();


        public void Init(int maxUserCount)
        {
            MaxUserCount = maxUserCount;
        }

        public ERROR_CODE AddUser(int sessionID)
        {
            if (UserSessionMap.ContainsKey(sessionID))
            {
                return ERROR_CODE.ADD_USER_DUPLICATION_SESSION;
            }


            UserSequenceNumber += 1;

            var user = new ConnectUser();
            user.Set(UserSequenceNumber, sessionID);

            UserSessionMap.Add(sessionID, user);

            return ERROR_CODE.NONE;
        }

        public ERROR_CODE RemoveUser(int sessionID)
        {
            if (UserSessionMap.Remove(sessionID) == false)
            {
                return ERROR_CODE.REMOVE_USER_SEARCH_FAILURE_SESSION;
            }

            return ERROR_CODE.NONE;
        }

        public ConnectUser GetUser(int sessionID)
        {
            ConnectUser user = null;
            UserSessionMap.TryGetValue(sessionID, out user);
            return user;
        }

        public ConnectUser GetUser(string userID)
        {
            ConnectUser user = null;
            UserIDMap.TryGetValue(userID, out user);
            return user;
        }

        public int GetConnectCount()
        {
            return UserSessionMap.Count();
        }

        public ERROR_CODE 유저_인증_완료(string userID)
        {
            var user = GetUser(userID);

            if (user == null)
            {
                return ERROR_CODE.USER_AUTH_SEARCH_FAILURE_USER_ID;
            }

            if (user.SetAuthorized() == false)
            {
                return ERROR_CODE.USER_AUTH_ALREADY_SET_AUTH;
            }

            return ERROR_CODE.NONE;
        }
    }


    class ConnectUser
    {
        Int64 SequenceNumber = 0;
        public int SessionID { get; private set; }
        public bool Authorized { get; private set; }
        public short LobbyID { get; private set; }


        public void Set(Int64 sequence, int sessionID)
        {
            Authorized = false;
            SequenceNumber = sequence;
            SessionID = sessionID;
        }

        public bool SetAuthorized()
        {
            if (Authorized)
            {
                return false;
            }

            Authorized = true;

            return true;
        }
    }
}
