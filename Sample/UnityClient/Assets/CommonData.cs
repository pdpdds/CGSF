using UnityEngine;
using System.Collections;

namespace CommonData
{
    #region Test
    public class ECHO
    {
        public string Msg1;
    }
    #endregion

    #region 요청 패킷 3만번대@
    //@brief : 로그인 요청@
    public class REQ_LoginInfo
    {
        public string sId;
        public string sPwd;
    }
    //@brief : 아이디 중복 확인@
    public class REQ_IdIsValid
    {
        public string sId;
    }
    //@brief : 가입 요청@
    public class REQ_Registration
    {
        public string sId;
        public string sPwd;
        public string sMail;
        public string sExtra;
        public string sUDID;
    }
    #endregion
    #region 응답 패킷 6만번대@
    public class RES
    {///모든 응답 패킷은 결과값을 가진다@
        public int iResult = - 1;
    }
    ////////////
    public class RES_LoginInfo : RES
    {/// = 0 : 성공 아이피등 전송 받음, = 1 :실패@
        public int iWorldCount;
        public string[] sIp;
    }
    public class RES_IdIsValid : RES
    {///RES = 0 : 성공, = 1 : 사용 불가, = 2 : 중복아이디@
    }
    public class RES_Regstration : RES 
    {
    }
    #endregion
}
