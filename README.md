# C++ IOCP 기반 서버 프로젝트

이 프로젝트는 **C++ IOCP 기반의 ServerCore**와 **Unity와 통신하는 GameServer**로 나뉘어져 있습니다.  
패킷 자동화를 위해 **Google Protocol Buffers**와 **Python 3.9**를 사용하였습니다.

---

## 📹 테스트 영상

[테스트 영상 보기](https://youtu.be/ouMp5Gto1_s)

---

## 📂 주요 디렉토리 구조

### `Common`
공용 파일들을 저장한 폴더입니다.  
- **Google Protocol Buffers**를 사용한 패킷 자동화 코드 포함.
- 클라이언트와 공유하는 **GridMap** 정보 저장.

### `Libraries`
외부 라이브러리 및 정적 라이브러리 파일들이 포함된 폴더입니다.  
- Google Protocol Buffers 라이브러리.
- ServerCore 정적 라이브러리.

### `PacketGenerator`
패킷 자동화를 위한 Python 스크립트가 포함된 폴더입니다.  
- Protobuf에 따라 사용해야 할 패킷을 자동 생성.

### `ServerCore`
IOCP 기반 소켓 통신을 위한 뼈대 라이브러리 구현 폴더입니다.  
다양한 서버 기능을 포함하고 있으며, 다른 서버에서 공용으로 사용됩니다.

### `Server_A`
클라이언트와 통신하는 **GameServer** 프로젝트 폴더입니다.

---

## 🛠️ ServerCore 주요 구성 요소

### **1. Main**
`GameServer`에서 공통으로 사용하는 파일들:
- **`Types.h`**: 자주 사용하는 타입 및 정의된 별칭 포함.
- **`pch`/`CorePch`**: 프로젝트 전역에서 자주 사용되는 헤더 포함.
- **`CoreMacro`**: 디버깅 및 동기화를 위한 유용한 매크로 정의 (`ASSERT_CRASH`, `LOCK`, 등).
- **`CoreGlobal`**: 전역 객체 정의:
  - **`GThreadManager`**: 스레드 관리.
  - **`GMemory`**: 메모리 할당 및 해제 관리.
  - **`GSendBufferManager`**: Chunk 방식으로 전송 버퍼 관리.
  - **`GDeadLockProfiler`**: 데드락 탐지 및 디버깅.
  - **`SocketUtils::Init()`**: Windows 소켓 초기화.

---

### **2. Memory**
효율적인 메모리 관리 기능:
- **`Allocator`**: 다양한 메모리 할당/해제 정책 정의 (`Heap`, `VirtualAlloc`, `MemoryPool`).
- **`Container.h`**: C++ STL 컨테이너를 커스터마이징하여 별칭으로 사용.
- **`Memory`**: 전역 `MemoryPool`을 통한 메모리 할당/해제 로직 구현.
- **`MemoryPool`**: 미리 준비된 메모리 공간을 활용하여 빈번한 메모리 할당/해제 최적화.

---

### **3. Thread**
스레드 및 동기화 관련 기능:
- **`Lock`**: `Atomic` 타입 기반 커스텀 Lock (`WriteLock`, `ReadLock`) 구현.
  - 정책: `W->R` 허용, `R->W` 금지.
- **`ThreadManager`**: 전역 스레드 관리.
- **`DeadLockProfiler`**:
  - **스레드 로컬 저장소**의 `LLockStack` 사용.
  - DFS 기반 역방향 간선 탐지를 통해 데드락 발생 여부 확인.

---

### **4. Network**
IOCP 기반 네트워크 통신:
- **`NetAddress`**: IP/Port 정보를 `sockaddr_in` 형식으로 변환하여 관리.
- **`SocketUtils`**: 비동기 소켓 통신을 위한 설정 및 초기화.
- **`IocpCore`**: IOCP Core CreateIoCompletionPort 관리 및  CreateIoCompletionPort 및 키 값과 overlapped를 상속받을 클래스 정의
- **`IocpEvent`**: **`OVERLAPPED`**를 상속받아 IOCP 이벤트 정의.
- **`Listener`**: 클라이언트 접속 요청 처리.
- **`ServerService`**: 세션 및 소켓 관리, 서버 상태 정보 제공.
- **`Session`**: 클라이언트 소켓과 통신 관리.
- **`RecvBuffer`**: 클라이언트로부터 데이터를 수신하는 버퍼.
- **`SendBuffer`**:
  - Chunk 방식의 전송 버퍼.
  - 각 스레드에서 스레드 로컬 저장소로 독립적chunk를 가지고있으며, 전역 **`SendBufferManager`**로 통합 관리.
 
## 🛠️ GameServer 주요 구성 요소
### **1. Uthis**
`Server_A`프로젝트에서 공통으로 사용할 기능 정의:
- **`pch`**: 자주 사용하는 타입 및 정의된 별칭 포함. serverCore 및 GoogleProtobuf 라이브러리 path 지정
---
### **2. Protocol**
`Google Protobuf`관련 패킷정의 파일 및 GoogleProtubuf 파일:
- **`Protocol.proto`**: Protobuf 패킷 정의 파일
- **`GenPackets.bat`**: Protobuf 패킷 정의 파일 및 Pyhon PactketGenerator파일을 이용하여 패킷관련 소스 자동화 batch파일(클라이언트가 C#이기 때문에 C#용 패킷파일도 같이 만들어 클라이언트 경로에도 추가)
- **`Protocol.pb.*`**: protobufc.exe파일로 생성된 Protobuf 헤더파일과 구현파일
---
### **3. Data**
서버와 클라이언트 공용으로 사용할 데이터 추출: nlohmann/json 라이브러리  사용
- **`DataManager`**: 몬스터 데이터 및 스킬 데이터 json으로 읽기,쓰기 및 외부클래스에 제공
- **`DataContent`**: json에서 읽은 몬스터 및 스킬데이터 를 담아 둘 객체 정의
- **`ConfigManager`**: 외부에서 미리 들고와야 할 json 및 파일들의 경로를 저장해둔 json파일을 가저와 설정
---
### **4. Main **
실질적으로 서비스를 오픈해 클라이언트와 소통을  하게 될 서버관련 파일
- **`server.cpp`**: Main 함수 , 프로그램의 시작점 
- **`GameSession`**: 현재 서비스에 필요한 세션의 기능을 ServerCore의 Packetsession을 상속 받아 정의 
- **`GameSessionManager`**: GameSession을 전역으로 관리할 매니저
- **`ClientPacketHandler`**: Protobuf 패킷을 사용하여 송수신 패킷들을 처리해주는 패킷자동화소스 파일 (python PacketGeneragor 에 의해 GenPackets.bat실행시 파일 자동 수정)
---
### **5. Game  **
  게임 컨텐츠 관련 파일
  #### **5.1 Object  **
  게임에서 사용 될 오브젝트 정의 
  - **`GameObject`**: Object들의 부모 클래스
  - **`ObjectManager`**: Object의 생성 및 삭제 또는상태 관리 등을 하는 매니저
  - **`Player`**: GameObject를 상속받은 Player 클래스
  - **`Monster`**: GameObject를 상속받은 Monster 클래스
  - **`ProtjectTile`**: GameObject를 상속받은 ProjectTile(투사체) 클래스
  - **`Arrow`**: ProjectTile를 상속받은 Arrow 클래스
  #### **5.2 Room  **
  사용자 및 게임 오브젝트가 생성되어 활동하게 될 Room 관련 클래스
  - **`Room`**: 사용자 및 게임 오브젝트가 생성되어 활동하게 될 Room 클래스
  - **`RoomManager`**: 여러개의 Room을 관리하게 될 클래스
  - **`MapManager`**: 클라이언트 와 같이 사용하는 타일맵의 데이터를 저장하고 Map에서 일어나는 일 들을 구현하는 클래스
  #### **5.3 Job  **
  서버에서 일어나는 일 들을 전부 Lock으로 처리시 일어나는 성능저하 및 상호배제로 인해 하나의 Job으로 만들어 비동기식으로 queue에 넣어 관리
  
  -- 진행중입니다. 현재 job queue를 이용하여 event등록 처리중--
  
---
## 🛠️ PacketGenerator 주요 구성 요소

  ### **PacketGenerator**
  - **`PacketGenerator`**: 템플릿을 읽어 ProtoParser에 의해 파싱된 데이터를 사용하여 템플릿을 만들어주는 파일
  - **`ProtoParser`**: .proto파일을 읽어 파싱해 줄 파일
---
## 📝 개발 환경
- **언어**: C++ 14, Python 3.9
- **네트워크 모델**: IOCP (I/O Completion Port)
- **프로토콜 자동화**: Google Protocol Buffers
- **클라이언트**: Unity

---
