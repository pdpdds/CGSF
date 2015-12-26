using UnityEngine;
using System.Collections;
using NETCODE;
using System.Security.Cryptography;
using System.Collections.Generic;
using System;
using System.Text;

public class ConnectScene : BaseScene
{
    
    
    int iCheckIDvalid = 0;//0 == 미확인, 1 == 사용가능, 2 == 사용불가
    bool bLogined = false;
    string sCheckedID = "";

    public override void Awake()
    {
        LoginNetManager.I.delegate_NonSystemReceiver += Onreceive;///1. 요청에 대한 응답 처리@
    }
    public override void Start () 
    {
	
	}
	public override void Update () 
    {
        LoginNetManager.I.Update();
	}
    public override void Destory()
    {
        LoginNetManager.I.delegate_NonSystemReceiver -= Onreceive;
    }

    private bool bLoginMode = false;
    
    private string sId ="foranie";
    private string spwd="";

    private bool bCheckMsg = false;

    private string sReg_id = "";
    private string sReg_pwd = "";
    private string sReg_re_pwd = "";
    private string sReg_email = "";
    #region built 1
    void OnGUI()
    {
        if ((bLoginMode == false)&&(bLogined == false))
        {
            if (GUI.Button(new Rect(10, 10, 100, 50), "LOGIN"))
            {
                bLoginMode = true;
                sReg_id = "";
                sReg_pwd = "";
                sReg_re_pwd = "";
                sReg_email = "";
            } 
            GUI.Box(new Rect(10, 120, 100, 30), "ID");
            sId = GUI.TextField(new Rect(120, 120, 200, 30), sId, 20); 
            
            GUI.Box(new Rect(10, 160, 100, 30), "PWD");
            spwd = GUI.TextField(new Rect(120, 160, 200, 30), spwd,20);
            if (GUI.Button(new Rect(330, 120, 100, 70), "LOGIN"))
            {
                LoginNetManager.I.REQL_Login(sId, spwd);
            }

            if (GUI.Button(new Rect(10, 250, 100, 40), "REQ_NO_ID"))
            {
                sId = "noID";
            }
            if (GUI.Button(new Rect(110, 250, 120, 40), "REQ_SERVER_FULL"))
            {
                sId = "serverFULL";
            }
            if (GUI.Button(new Rect(230, 250, 120, 40), "REQ_UNKNOWN"))
            {
                sId = "etcError";
            }

        }
        else if ((bLoginMode == true) && (bLogined == false))
        {
            if (GUI.Button(new Rect(10, 10, 100, 50), "REGISTER"))
            {
                bLoginMode = false;
                sId = "";
                spwd = "";
            }
            GUI.Box(new Rect(10, 120, 100, 30), "ID");
            sReg_id = GUI.TextField(new Rect(120, 120, 200, 30), sReg_id, 20);

            GUI.Box(new Rect(10, 160, 100, 30), "PWD");
            sReg_pwd = GUI.TextField(new Rect(120, 160, 200, 30), sReg_pwd, 20);

            GUI.Box(new Rect(10, 200, 100, 30), "REPWD");
            sReg_re_pwd = GUI.TextField(new Rect(120, 200, 200, 30), sReg_re_pwd, 20);
            
            GUI.Box(new Rect(10, 240, 100, 30), "EMAIL");
            sReg_email = GUI.TextField(new Rect(120, 240, 200, 30), sReg_email, 20);

            if (GUI.Button(new Rect(330, 120, 50, 30), "Check"))
            {
                LoginNetManager.I.REQL_IDisValid(sReg_id);
            }

            if (GUI.Button(new Rect(10, 300, 370, 40), "Register"))
            {
                if (sReg_pwd.CompareTo(sReg_re_pwd) == 0)
                {
                    //LoginNetManager.I.REQL_Registration(sReg_id, sReg_pwd, sReg_email, "", "");
                    LoginNetManager.I.REQL_Registration(this.sReg_id, sReg_pwd, sReg_email, "UNITY", SystemInfo.deviceUniqueIdentifier);
                }
                else if ((sReg_pwd == "") || (sReg_id == "") || (sReg_re_pwd == "") || (sReg_email == ""))
                {//공백@
                    bCheckMsg = true;
                    if(IsInvoking("Toast_Off") == false)
                        Invoke("Toast_Off", 5);
                }
                else
                {//두개가 같은지@
                    bCheckMsg = true;
                    if (IsInvoking("Toast_Off") == false)
                        Invoke("Toast_Off", 5);
                }
            }
        }
        
        if (bLogined == true)
        {
            if (GUI.Button(new Rect(10, 10, 100, 50), "LOGOUT"))
            {
                bLoginMode = false;
                bLogined = false;
                sId = "";
                spwd = "";
            }
        }
        if (bCheckMsg == true)
        {
            GUI.Box(new Rect(10, 350, 370, 100), "내용을 확인하시오");
        }
    }
    void Toast_Off()
    {
        bCheckMsg = false;
    }
    void req_registration()
    {
        if (iCheckIDvalid == 0)
        {
            Cloger.WR("중복확인 하세요");
            return;
        }
        else if (iCheckIDvalid == 2)
        {
            Cloger.WR("사용불가한 아이디");
            return;
        }


        string sID = "";
        string sPwd = "";
        string sMail = "";
        string sExtra = "";
        string sUDID = "";
        LoginNetManager.I.REQL_Registration(sID, sPwd, sMail, sExtra, sUDID);
    }
    #endregion

    #region scenefunction
    void CheckIDvalid(string sID)
    {
        if (sCheckedID == sID)
        {
            Cloger.WR("사용가능");
            return;
        }
        else
        {
            LoginNetManager.I.REQL_IDisValid(sID);
        }
    }
    
    #endregion
    public void Onreceive(jPacket packet)
    {
        switch (packet.Protocol)
        {
            case CommonProtocol.REQ_ECHO:
                //CommonData.ECHO echo = jPacket.MakeObject<CommonData.ECHO>(packet.JsonFormatData);
                //DebugOverlay.Instance.AddViewportText("RET_ECHO" + DateTime.Now.ToString(), 20f);
                //Invoke("invECHO", 2.0f);
                break;
            case CommonProtocol.KEEPALIVE:

                break;
            case CommonProtocol.RES_LOGIN:
                res_Login(packet);
                break;
            case CommonProtocol.RES_IDISIDVALID:
                res_IdIsValid(packet);
                break;
            case CommonProtocol.RES_REGISTRATION:
                res_Registration(packet);
                break;
            case CommonProtocol.RES_DISCONNECTED:
                break;
        }
    }
    #region packetParse
    public void res_Registration(NETCODE.jPacket packet)
    {
        CommonData.RES_Regstration idr = jPacket.MakeObject<CommonData.RES_Regstration>(packet.JsonFormatData);
        if (idr.iResult == 0)
        {
            Cloger.WR("아이디.생성.성공적");
            bLoginMode = false;
            
        }
        else if (idr.iResult == 1)
        {
            Cloger.WR("UDID 중복");
        }
        else if (idr.iResult == 2)
        {
            Cloger.WR("아이디 중복");
            iCheckIDvalid = idr.iResult;
        }
    }
    public void res_IdIsValid(NETCODE.jPacket packet)
    {
        CommonData.RES_IdIsValid idv = jPacket.MakeObject<CommonData.RES_IdIsValid>(packet.JsonFormatData);
        if (idv.iResult == 0)
        {
            Cloger.WR("사용가능");
            iCheckIDvalid = idv.iResult;
        }
        else if (idv.iResult == 1)
        {
            Cloger.WR("사용불가");
            iCheckIDvalid = idv.iResult;
        }
    }
    public void res_Login(NETCODE.jPacket packet)
    {
        CommonData.RES_LoginInfo login = jPacket.MakeObject<CommonData.RES_LoginInfo>(packet.JsonFormatData);
        //CommonData.RES_LoginInfo login = new CommonData.RES_LoginInfo();
        if (login.iResult == 1)
        {
            Cloger.WR("없는 아이디/틀린아이디");
        }
        else if (login.iResult == 3)
        {
            Cloger.WR("서버 꽉참 재시도 대기");
        }
        else if (login.iResult == 4)
        {
            Cloger.WR("알 수 없는 에러");
        }
        else if (login.iResult == 0)
        {
            bLogined = true;
            Cloger.WR("로그인 성공");
        }
    }
    #endregion
}
