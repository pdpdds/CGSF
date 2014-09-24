using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace CSCommonLib
{
    public enum PACKET_ID : ushort
    {
        INVALID             = 0,

        SYSTEM_CLIENT_CONNECT       = 11,
        SYSTEM_CLIENT_DISCONNECTD   = 12,

        REQUEST_LOGIN = 1101,
        RESPONSE_LOGIN = 1102,

        REQUEST_ENTER_LOBBY = 1111,
        RESPONSE_ENTER_LOBBY = 1112,

        REQUEST_LEAVE_LOBBY = 1121,
        RESPONSE_LEAVE_LOBBY = 1122,

        REQUEST_CHAT = 1131,
        NOTICE_CHAT = 1132,


        
        // SERVER SIDE
        DB_REQUEST_LOGIN = 2001,
        DB_RESPONSE_LOGIN = 2002,
    }

    public enum ERROR_CODE : ushort
    {
        NONE    = 0,

        ERROR = 1001,

        // 로그인 
        LOGIN_INVALID_AUTHTOKEN             = 10001, // 로그인 실패: 잘못된 인증 토큰
        ADD_USER_DUPLICATION_SESSION        = 10002,
        REMOVE_USER_SEARCH_FAILURE_USER_ID  = 10003,
        REMOVE_USER_SEARCH_FAILURE_SESSION  = 10004,
        USER_AUTH_SEARCH_FAILURE_USER_ID    = 10005,
        USER_AUTH_ALREADY_SET_AUTH = 10006,

        
        //ENTER_LOBBY_ALREADY_LOBBY = 10011,
        //ENTER_LOBBY_INVALID_LOBBY_ID = 10012,
        //ENTER_LOBBY_LOBBY_FULL = 10013,

        //LEAVE_LOBBY_DO_NOT_ENTER_LOBBY = 10021,
        //LEAVE_LOBBY_NO_LOBBY = 10022,

        //LOBBY_CHAT_DO_NOT_ENTER_LOBBY = 10031,

    }


    public struct JsonPacketRequestLogin
    {
        public string ID;
    }

    public struct JsonPacketResponseLogin
    {
        public ERROR_CODE Result;
    }


    public struct JsonPacketRequestEnterLobby
    {
        public short LobbyID;
    }

    public struct JsonPacketResponseEnterLobby
    {
        public ERROR_CODE Result;
        public short LobbyID;
    }


    public struct JsonPacketRequestLeaveLobby
    {
        public short LobbyID;
    }

    public struct JsonPacketResponseLeaveLobby
    {
        public ERROR_CODE Result;
    }


    public struct JsonPacketRequestChat
    {
        public string Chat;
    }

    public struct JsonPacketNoticeChat
    {
        public ERROR_CODE Result;
        public string UserID;
        public string Chat;
    }
}
