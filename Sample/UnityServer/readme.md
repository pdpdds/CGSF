**UnitySimpleLogin**

Chat 예제를 응용하였고 컨텐츠 외에 수정된 것은 없습니다.
ProcessPacket()에서 각각의 프로토콜을 인식하여 switch-case로 처리합니다.
요청을 받는 프로토콜은

const static USHORT REQ_LOGIN = 30001;///로그인 요청@
const static USHORT REQ_IDISIDVALID = 30002;///아이디 중복확인@
const static USHORT REQ_REGISTRATION = 30003;///신규 가입 요청@

세가지가 있고 응답은

const static USHORT RES_LOGIN = 60001;///로그인 응답@
const static USHORT RES_IDISIDVALID = 60002;///아이디 중복응답@
const static USHORT RES_REGISTRATION = 60003;//신규 가입 응답@

이렇게 세가지가 있습니다. 유니티 클라 예제에 비해 특별히 설명할건 없는 것 같고 의식의 흐름에 따라 코드를 풀어서 기술하였기에 어려운 것도 없을 것 같습니다. 간략히 보자면...




**함수별 설명**

30001

처리는 REQD_LOGIN()함수가 맡습니다.
11번,12번 라인에서 아이디 패스워드를 읽어옵니다. md5등으로 변환했다면 해제하는 과정이 필요하겠으나 생략.
18번 라인에서는 아이디와 패스워드를 검사합니다. std::list에 UserInfo형태로 통째로 저장되어 있습니다. 유저 등록을 하면 리스트에 추가 됩니다. DB를 읽거나 하는 부분은 예제의 의도에서 벗어나니 생략.
iPwdChk =  0이면 로그인 가능
iPwdChk = 1이면 리스트에 없는 아이디

테스트 아이디

foranie가 전송하면 무조건 성공을 반환 합니다.
noID가 전송하면 iPwdChk == 1을 반환합니다.
serverFULL을 전송하면 iPwdChk == 3을 반환 합니다.
...

30002

처리는 REQD_IDISIDVALID()함수가 맡습니다.
77번,78번에서 전송 받은 아이디가 std::list에 있는지 확인하여 0 또는 1을 반환 합니다.


30003

처리는 REQD_REGISTRATION()함수가 맡습니다.
예외처리는 아이디 중복 외에 전혀 안되어 있고 중복이 아닌 경우 리스트에 등록 됩니다.




**마무리**

대부분의 사람들이 몰라서가 아니라 처음부터 작성하기 귀찮아서 예제 코드를 찾아다닐 확률이 크다는 생각이 들어 단계별로 예제를 만들 생각입니다.

 
