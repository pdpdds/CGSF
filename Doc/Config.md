#네트워크 설정
 * 설정 파일
  * CGSF의 네트워크 설정은 EngineConfig.xml 을 통해서 설정된다.
  * EngineName: 네트워크 엔진 dll 이름.
  * ServerIP: IP 주소. 대부분 0.0.0.0 이나 127.0.0.1을 사용하면 된다.
  * ServerPort: 포트 번호
  * MaxAcceptCount: 최대 접속자 수. 이 수는 Connector, Multi Listen을 모두 포함한 수.
     * Connector은 다른 서버에 접속하는 경우,  Multi Listen는 2개 이상의 Port를 사용하는 경우
     * 사용할 수 있는 최대 수는 65535. 코드 상에서 MAX_CGSF_CONCURRENT_USER에 선언되어 있다.



##C++


##.NET Framework


