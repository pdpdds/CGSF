## 위치
CGSF 베이스 디렉토리에서 **NETFramework** 디렉토리에 닷넷 관련 코드들이 있다.
 * CgsfNET64LibProject: CGSF의 C++ 라이브러리를 C++/CLI를 사용해서 만든 CGSF 닷넷 라이브러리
 * CGSFNETCommon: CGSF.NET 공통 라이브러리
 * Lib: CGSF.NET에 필요한 라이브러리가 있는 디렉토리
    * CgsfNET64Lib: CgsfNET64LibProject에서 빌드된 라이브러리가 있는 장소
    * CGSFNETCommon: CGSFNETCommon에서 빌드된 라이브러리가 있는 장소
 * Lib list.txt: CGSF.NET 라이브러리 사용을 위해 필요한 외부 라이브러리 리스트가 기록  
 

## 빌드
 * Visual Stuio 2013
 * .NET Framework 4.5.1


## 사용
 1. NETFramework/CgsfNET64LibProject 디렉토리에 있는 **CgsfNET64Lib.sln**을 빌드한다.
 2. NETFramework/CGSFNETCommon 디렉토리에 있는 **CGSFNETCommon.sln**을 빌드한다.
 3. 1,2번 프로젝트를 빌드해서 생성된 파일은 NETFramework\Lib 디렉토리에 각각의 디레렉토리에 있누다.**CgsfNET64Lib.dll**, **CGSFNETCommon.dll**
 4. C# 프로젝트를 만든다(여기서는 Echo 프로젝트를 만든다)
 5. 참조로 CgsfNET64Lib.dll, CGSFNETCommon.dll을 추가한다.
 6. CGSF.NET 라이브러리를 사용하여 네트워크 초기화 및 시작을 한다.
 ```c#
 Config = new CgsfNET64Lib.NetworkConfig()
 {
       IP = "127.0.0.1",
	   Port = 25251,
	   EngineDllName = "CGSFNet.dll",
	   MaxAcceptCount = 1000,
	   ThreadCount = 4,
	   ProtocolOption = 0,
	   ProtocolID = 0,
	   MaxBufferSize = 16000,
	   MaxPacketSize = 4012,
 };

 ServerNetwork ServerNet = new ServerNetwork();
 if(ServerNet.Init(Config, null, null) == CgsfNET64Lib.NET_ERROR_CODE_N.SUCCESS)
 {
 	// 성공
 }
 ```
 7. 패킷 처리
 ```c# 
	var packet = ServerNet.GetPacket();
	if (packet == null)
	{
		return;
	}

	switch (packet.GetPacketType())
	{
		case CgsfNET64Lib.SFPACKET_TYPE.CONNECT:
			// 접속
			break;
		case CgsfNET64Lib.SFPACKET_TYPE.DISCONNECT:
			// 접속 끊어짐
			break;
		case CgsfNET64Lib.SFPACKET_TYPE.DATA:
			// 클라이언트가 보낸 데이터
			break;
 ```
 8. 패킷 보내기
 ```c# 
  var bodyData = JsonEnDecode.Encode<JsonPacketNoticeEcho>(request);
  ServerNet.SendPacket(packet.SessionID(), PACKET_ID_ECHO, bodyData);
 ```
 9. 종료
 ```c#
  ServerNet.Stop();
 ```