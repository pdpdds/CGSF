using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace ChatServer1
{
    enum PACKET_ID : ushort
    {
        SYSTEM_DISCONNECTD = 12,

        REQUEST_LOGIN   = 1001,
        RESPONSE_LOGIN  = 1002,

        REQUEST_ENTER_LOBBY = 1011,
        RESPONSE_ENTER_LOBBY = 1012,

        REQUEST_LEAVE_LOBBY = 1021,
        RESPONSE_LEAVE_LOBBY = 1022,

        REQUEST_CHAT = 1031,
        NOTICE_CHAT = 1032,
    }

    enum ERROR_CODE : ushort
    {
        ERROR   = 0,

        ADD_USER_DUPLICATION_SESSION = 1001,
        ADD_USER_DUPLICATION_ID = 1002,

        NONE    = UInt16.MaxValue
    }


    struct JsonPacketRequestLogin
    {
        public string ID;
    }

    struct JsonPacketResponseLogin
    {
        public ERROR_CODE Result;
    }


    struct JsonPacketRequestEnterLobby
    {
        public short LobbyID;
    }

    struct JsonPacketResponseEnterLobby
    {
        public ERROR_CODE Result;
        public short LobbyID;
    }


    struct JsonPacketRequestLeaveLobby
    {
        public short LobbyID;
    }

    struct JsonPacketResponseLeaveLobby
    {
        public ERROR_CODE Result;
    }


    struct JsonPacketRequestChat
    {
        public string Chat;
    }

    struct JsonPacketNoticeChat
    {
        public ERROR_CODE result;
        public string ID;
        public string Chat;
    }
}
