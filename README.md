
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
    
    - Types.h : 자주 사용하게 되는 타입 및 정의를 별칭으로 묶어 둔 파일
    - pch : CorePch를 참조 파일
    - CorePch : GameServer 및 ServerCore에서 자주 사용하게 될 헤더 파일
    - CoreMacro : 자주 사용하게 될 Lock 과 의도적인 CRASH 및 ASSERT_CRASH를 사용하여 디버그 환경에서 조건에 따라 CRASH를 사용 매크로 정의 
    - CoreGlobal : 전역으로 사용하게 될 글로벌 객체들의 정의 파일
      - 전체 스레드를 관리하는 GThreadManager     
      - 데이터의 할당 및 해제를 관리하는 GMemory
      - SendBuffer를 chunk 방식을 사용하여 SendBuffer를 관리하는 사용하는 GSendBufferManager
      - 디버그 환경에서 데드락이 발생할 요소가있는 부분을 관리하는 GDeadLockProfiler 
      - 논블락 비동기 소켓 통신시 런타임때  미리 받아와야 할 connect , disconnect, accept 등을 초기화하는 SocketUtils::init();
      - 
  - Memory
      - Allocator : 기본 heap메모리 할당 해제 , virtualAlloc 및 virtualFree 사용한 할당과 해제 , MemoryPool을 사용하는 할당과 해제, 멀티스레드 환경에서 STL의 할당과 해제 등의 정의 
      - Container.h : C++ 표준 stL의 오버로딩 된 것을 이용하여 해당 STL을 할당 및 해제할때 사용할 할당 및 소멸을 지정하는 별칭으로 사용을 정의한 파일.
      - Memory :  메모리 풀 을 사용하여 할당과 해제를 메모리풀에서 할당과 해제를 하게 사용 전역변수의 GMemory에서 객체의 할당과 해제를 로직을 정의한 파일
      - MemoryPool : Memory에서 MemoryPool에서 정의 된 push pop을 이용하여 지정된 크기의 할당 및 해제는 바로 할당 및 해제하는 것이 아닌 빈번하게 일어날경우를 대비하여 미리 준비해둔 공간에 할당 및 해제
   - Thread
      - lock : 표준 lock을 사용하는 것이아닌 Atomic type을 이용하여 만든 writelock 과 readlock을 정의 한 파일 만들어 사용하는 lock이기에 정책또한 정의되어있음 ex) W->R (O) R->W (X) 결론적으로 lock이 아닌 atomic으로 사용하기 위한 커스텀 lock
      - ThreadManager : 전역으로 사용할 스레드를 관리하는 매니저 정의
      - DeadLockProfiler : 스레드 로컬 저장소의 LLockStack을 이용하여 Debug단계에서 그래프의 DFS깊이 우선 탐색 알고리즘을 역방향 간선을 찾아 이용하여 데드락 발생 가능성이 있을시 CRASH를 호출
  - Network
      - NetAddress : IP 및 port를 windows socekt에서 사용하기위한 sockaddr_in 구조체로 바꾸어 관리  
      - SocketUtils : 논블락 비동기 통신 소켓을 위한 설정 및 소켓이벤트 정의 및 특정 이벤트 비동기로 사용하기위한 함수포인터 정의
      - IocpCore : createIOCompletionPort의 키 값을 만들어 줄 IocpObject 클래스와 CreateIoCompletionPort를 가지고있을 IocpCore를 가지고 있을 클래스 정의
      - IocpEvent : OVERLAPPED를 상속받은 IocpEvent 클래스 정의
      - Listenr : 클라이언트의 접속 요청을 처리해줄 Listener 소켓 정의 
      - ServerService : 메인 Server가 되어 session , Listener 소켓 관리 및 서버의 상태정보를 관리
      - Session : 클라이언트 의 정보 및 클라이언트 와 통신(recv,send)을 주고받는 소켓을 들고있는 클래스 
      - RecvBuffer : 클라이언트가 전송한 데이터를 담아 둘 RecvBuffer 정의 클래스
      - SendBuffer :  클라이언트에게 데이터를 보내기위한 버퍼 정의 클래스. (각 스레드마다 들고있는 Sendbufferchunk에서 메모리를 조금식 할당받아 사용 전역으로 선언된 SendBufferManager에서 관리)
        



