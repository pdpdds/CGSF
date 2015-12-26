*Unity Client

요약
쓰레드를 쓰는 방법과 비동기 함수를 이용한 방법 두가지 구현중 비동기 함수를 이용 했습니다.
로그인 후 각각의 이동을 동기화 하는 수준까지 하려고 했으나 이걸 못하는 분은 이걸 볼 일이 없다는 결론에 도달하여 귀찮음을 줄여주는 수준까지 역으로 추려내어 올립니다.
내부 구현의 차이에 대해 언급해두었고 문제가 있다면 지적및 수정 부탁 드립니다.



주요 클래스 요약
NetManager_V2 : 서버당 하나의 연결을 담당 합니다. 여기에 connect, recvBuffer,sendBuffer가 있습니다.
LoginNetManager : 만약 여러개의 서버를 쓰는 경우 통합관리를 할 부분입니다. (예제 에서는 하나)
여러개의 NetManager_V2를 리스트로 관리하며 유니티 씬의 어느 한 객체의 Update()에서 Update()를 호출 해줘야 합니다.
구현에 따라서 요청 메세지를 이 클래스에 메소드로 구현하여도 좋지만 (예제에서는 여기 모두 구현)
OOP에 맞게 연결하는 서버 혹은 역활에 따라 분리하여 관리하는것이 더 좋습니다. 

CommonData : 로그인 요청, 아이디 중복 확인 요청 패킷과 응답 패킷 구조체가 구현되어 있습니다. 
응답의 경우 전체 요청에 대한 결과 값 int iResult를 멤버로 가지는 자식들로 이루어져 있습니다.

CommonProtocol : UINT16으로된 프로토콜이 정의되어 있습니다.

JPacket : CGSFJSON패킷이 정의되어 있습니다. 완전히 동일한 헤더를 가지고 있으므로 헤더를 수정하신 경우 참고 해주시기 바랍니다..
{//12
    public UInt16 Protocol;//2
    public UInt32 PacketOption;//4
    public UInt32 DataCRC;//4
    public UInt16 DataSize;//2
    public const int HEADERSIZE = 12;
}

BaseScene : Mono를 상속받은 Scene객체 입니다. 모든 씬이 상속 받아서 구현하도록 되어있고 전역적으로 메세지를 받아서 처리해야하는 것이 있다면 여기서 정의합니다. 
(하트 라던지.. 채팅 메세지라던지..)
ConnectScene : 실질적으로 컨텐츠가 들어가는 씬 입니다.(자세한 것은 아래 설명)

Clog : 지우셔도 됩니다. debug.log를 화면에 띄워주기위한 역활입니다.


씬의 초기화와 예제의 구성

void Awake()
17줄 Receive를 받을 함수를 지정합니다. 여기 등록된 함수 전에 LoginNetManager 59줄에 먼저 걸리는데 
KeepAlive(),또는 heartbeat()등 컨텐츠 레벨까지 내려갈 필요가 없는 경우 여기서 걸러주면 됩니다.


void Update()
25줄 LoginNetManager.I.Update(); 버퍼에 들어있는 값이 있는지 검사하여 처리합니다. 이부분이 빠지면 패킷을 받아도 처리를 하지 못합니다.

void OnDestory()
29줄 연결을 끊어줍니다.
30줄 LoginNetManager의 메세지 처리 부분을 끊어줍니다. 



메세지 요청
LoginNetManager.I.REQL_Login(sId, spwd);
LoginNetManager.I.REQL_IDisValid(sReg_id);
LoginNetManager.I.REQL_Registration(this.sReg_id, sReg_pwd, sReg_email, "UNITY", SystemInfo.deviceUniqueIdentifier);

상단의 2번 항목에서 언급하였듯 여기서는 LoginNetManager에서 요청 메세지 작성까지 담당하지만 좀 더 복잡한 프로그램이 된다면 분리하셔야 합니다..


메세지 응답
3번 항목의 Awake()에서 등록한 Onreceive()함수로 응답이 돌아옵니다.
switch (packet.Protocol)
{
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
구현은 각각 참조 해주시기 바랍니다.


테스트 방법
접속 주소 127.0.0.1:25251
서버 : UnityServer내부의 BIN으로 이동, UnitySimpleLoginServer.exe파일을 VSProject/32로 옮겨서 실행.
클라 : 받은 Asset폴더로 프로젝트를 구성하고 씬을 로드하여 실행 하거나 UnityClient내부의 BIN폴더로 이동. UnitySample을 실행.

상단의 LOGIN 버튼을 클릭하면 아이디 등록폼으로 바뀝니다. 등록시 로컬에서 패스워드가 같은지 확인하지는 않습니다.
등록 폼에서 등록이 끝나면 로그인으로 이동하여 로그인이 됩니다.





마치며
다른 부분을 신경쓰지 않고 동작법을 눈대중으로 알 수 있도록 한 예제라 당연하지만 절대 이대로 구현하지 마시기 바랍니다.
가령 로그인 서버라면 잘못된 로그인 요청이었을 때 틀렸으면 응답 후 소켓 연결을 끊는게 일반적 입니다.
패스워드를 암호화 하여 전송하지도 않았습니다. 서버측 연결 끊음 등에 대한 대응도 스스로 하셔야 합니다.
