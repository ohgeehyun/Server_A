
- ### 프로젝트 소개

  c++ iocp 기반의 ServerCore 부분 과 실질적으로 unity와 통신하는 GameServer 부분 으로 나뉘어진 서버. 패킷자동화 관련 부분은 Google ProtoBuffer 와 python3.9 부분을 사용하여 패킷 자동화 부분 적용

- ## 테스트 영상 링크
 
  https://youtu.be/ouMp5Gto1_s
------------------------------------------------------------------

- ### 주요 Directory 

  - Common : 공용 파일들을 저장하여 사용 Google Buffer를 사용한 패킷 자동화 및 클라이언트 와 같이 사용하게 될 GridMap에 대한 정보가 담겨있다.

  - Libraries : 외부 라이브러리(GoogleProtoBufFer) 및 ServerCore 라이브러리를 정적라이브러리로 담아 둔 폴더.

  - PacketGenerator : Protobuf에 따라 사용할 패킷이 늘어남에 따라 packethandler에서 패킷만드는 작업을 자동화 해주는 python 파일 폴더

  - ServerCore : iocp socket통신을 하기 위한 뼈대 라이브러리들을 작성한 폴더 

  - Server_A : 실질적으로 클라이언트와 통신을할 서버의 파일 폴더
  
-------------------------------------------------------------------

- ### SeverCore
  - Main : GameServer에서 공통으로 사용하는 파일들
    - Types.h : 자주 사용하게 되는 타입 및 정의를 namespace로 묶어 둔 파일
    - pch : CorePch를 참조 파일
    - CorePch : GameServer 및 ServerCore에서 자주 사용하게 될 헤더 파일
    - CoreMacro : 자주 사용하게 될 Lock 과 의도적인 CRASH 및 ASSERT_CRASH를 사용하여 디버그 환경에서 조건에 따라 CRASH를 사용 매크로 정의 파일
    - CoreGlobal : 전역으로 사용하게 될 글로벌 객체들의 정의 파일
      - 전체 스레드를 관리하는 GThreadManager
      
      - 데이터의 할당 및 해제를 관리하는 GMemory
      
      - SendBuffer를 chunk 방식을 사용하여 SendBuffer를 사용하는 GSendBufferManager
      
      - 디버그 환경에서 데드락이 발생할 요소가있는 부분을 관리하는 GDeadLockProfiler
      
      - 논블락 비동기 소켓 통신시 런타임때  미리 받아와야 할 connect , disconnect, accept 등을 초기화하는 SocketUtils::init();
  - Memory
        
      



