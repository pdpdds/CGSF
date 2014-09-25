using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace ChatServerLib
{
    class ConnectUser
    {
        Int64 SequenceNumber = 0;
        public int SessionID { get; private set; }
        public string UserID { get; private set; }
        public CONNECT_USER_STATE CurrentState { get; private set; }
        public short LobbyID { get; private set; }


        public void Set(Int64 sequence, int sessionID)
        {
            BasicState();
            SequenceNumber = sequence;
            SessionID = sessionID;
        }

        public void BasicState()
        {
            UserID = "";
            CurrentState = CONNECT_USER_STATE.NONE;
        }

        public void TryAuth()
        {
            CurrentState = CONNECT_USER_STATE.ATUH_WAIT;
        }

        public bool SetAuthorized(string userID)
        {
            if (CurrentState != CONNECT_USER_STATE.ATUH_WAIT)
            {
                return false;
            }

            UserID = userID;
            CurrentState = CONNECT_USER_STATE.ATUH_COMPLETE;

            return true;
        }

        public void TryEnterLobby()
        {
            CurrentState = CONNECT_USER_STATE.LOBBY_WAIT;
        }

        public void SetLobby(short lobbyID)
        {
            if (lobbyID > 0)
            {
                CurrentState = CONNECT_USER_STATE.LOBBY;
            }
            else
            {
                CurrentState = CONNECT_USER_STATE.ATUH_COMPLETE;
            }

            LobbyID = lobbyID;
        }
    }

    enum CONNECT_USER_STATE
    {
        NONE            = 0,
        ATUH_WAIT       ,
        ATUH_COMPLETE   ,
        LOBBY_WAIT      ,
        LOBBY           ,
    }
}
