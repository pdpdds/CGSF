using UnityEngine;
using System.Collections;
using NETCODE;
using System.Security.Cryptography;
using System.Net.Sockets;
public class LoginNetManager
{
    /// <summary>
    /// 로그인 서버에서 할 수 있는 요청들만 담는다.
    /// 서버종류가 늘어나면 스테틱이 아니라 사전으로 관리한다.
    /// </summary>
    private bool bCallOnce = false; 
    private NetManager_V2 nm = null;
    private static LoginNetManager ins = null;
    public NetManager_V2.DELEGATE_Receive delegate_NonSystemReceiver = null;
    public static LoginNetManager I
    {
        get
        {
            if (ins == null)
            {
                ins = new LoginNetManager();
            }
            return ins;
        }
    }
    public LoginNetManager()
    {
        if (bCallOnce == false)
        {
            nm = new NetManager_V2("127.0.0.1",25251);
        }
    }
    public bool Connect()
    {
        try
        {
            if (nm.Connect() == false)
            {
                Cloger.WR("Connect ERROR");
                return false;
            }
            nm.delegate_Receive += ReadLoginPacketProc;
            bCallOnce = true;
            return true;
        }
        catch (SocketException se)
        {
            Cloger.WR(se.ToString());
            return false;
        }
    }
    public void Update()
    {
        if (nm != null)
            if (nm.IsConnected())
                nm.Runing();
    }
    public void ReadLoginPacketProc(jPacket packet)
    {
        switch (packet.Protocol)
        {
            default:
                delegate_NonSystemReceiver(packet);
                break;
        }

    }
    private void MSG_ERROR_CONNECT()
    {
        Cloger.WR("ERROR : NOCONNECT");
    }
    public void REQL_ECHO()
    {
        if (nm.IsConnected() == false) Connect();

        CommonData.ECHO echo = new CommonData.ECHO();
        jPacket packet = new jPacket(NETCODE.CommonProtocol.REQ_ECHO);
        nm.Send(packet.MakePacket<CommonData.ECHO>(echo));
    }
    public void REQL_Login(string sId, string sPw)
    {
        if (nm.IsConnected() == false) { if (Connect() == false) return; }

        CommonData.REQ_LoginInfo login = new CommonData.REQ_LoginInfo();
        login.sId = sId; login.sPwd = sPw;
        jPacket packet = new jPacket(NETCODE.CommonProtocol.REQ_LOGIN);
        nm.Send(packet.MakePacket<CommonData.REQ_LoginInfo>(login));
    }
    public void REQL_Registration(string sID, string sPwd, string sMail, string sExtra,string sUDID)
    {
        if (nm.IsConnected() == false) { if (Connect() == false) return; }
        CommonData.REQ_Registration rg = new CommonData.REQ_Registration();
        rg.sId = sID;
        rg.sPwd = sPwd;
        rg.sMail = sMail;
        rg.sExtra = sMail;
        rg.sUDID = sUDID;
        jPacket packet = new jPacket(NETCODE.CommonProtocol.REQ_REGISTRATION);
        nm.Send(packet.MakePacket<CommonData.REQ_Registration>(rg));
    }
    public void REQL_IDisValid(string sID)
    {
        if (nm.IsConnected() == false) { if (Connect() == false) return; }
        CommonData.REQ_IdIsValid Id = new CommonData.REQ_IdIsValid();
        Id.sId = sID;
        jPacket packet = new jPacket(NETCODE.CommonProtocol.REQ_IDISIDVALID);
        nm.Send(packet.MakePacket<CommonData.REQ_IdIsValid>(Id));
    }
    public void DisConnect(string sName)
    {
        //리스트에서 삭제@
    }
    public void DisConnectAll()
    {
        nm.Disconnect();
    }
}






