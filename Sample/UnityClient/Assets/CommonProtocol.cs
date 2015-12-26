using UnityEngine;
using System.Collections;
using System;
namespace NETCODE
{
    public class CommonProtocol
    {
        /* 임의로 나눴다.
         * 시스템/테스트 패킷은 1001이하에 배정@
         * 요청-req패킷은 3만번대에 배정@
         * 응답-response패킷은 6만번대에 배정@
         * 
         */
        public const UInt16 RES_DISCONNECTED = 1; 
        public const UInt16 REQ_ECHO = 1000;
        public const UInt16 RET_ECHO = 1001;
        public const UInt16 KEEPALIVE = 2000;//연결 유지@
        public const UInt16 REQ_HELLo = 30000;
        public const UInt16 RES_HELLo = 60000;


        public const UInt16 REQ_LOGIN = 30001;///로그인 요청@
        public const UInt16 REQ_IDISIDVALID = 30002;///아이디 중복확인@
        public const UInt16 REQ_REGISTRATION = 30003;///신규 가입 요청@

        
        public const UInt16 RES_LOGIN = 60001;///로그인 응답@
        public const UInt16 RES_IDISIDVALID = 60002;///아이디 중복응답@
        public const UInt16 RES_REGISTRATION = 60003;//신규 가입 응답@

        
    }
}
