# C++ IOCP 기반 서버 프로젝트

이 프로젝트는 **C++ IOCP 기반의 ServerCore**와 **Unity와 통신하는 GameServer**로 나뉘어져 있습니다.  
패킷 자동화를 위해 **Google Protocol Buffers**와 **Python 3.9**를 사용하였습니다.
**DB**의 경우 Mysql과 Redis 를 사용하였지만 iocp서버에서는 통신 중에 DB관련 네트워크 작업으로 인한 작업의 대기 , DB 응답 대기 부분 및 DB 데이터 처리를 최소화 하고자 **nodejs** 서버에게 DB 관련 일을 분배 하여 IOCP서버에서는 node.js에 request 하거나 서버가 맨 처음 실행시에만 DB관련 데이터를 읽어오는 방향으로 구현 하였습니다.

각 서버들의 물리적인 위치는 
A host ip server
-IOCP , Node.js
B host ip server(Aws ec2 사용)
-Mysql , Redis , Nginx(proxy) 로 구성되어 있으며

모든 통신은 **Nginx**를 거쳐 http통신의 경우 node.js 서버에 , TCP통신의 경우 IOCP로 서버에게 요청이 가도록 되어 있습니다.

---

## 📹 테스트 영상

[테스트 영상 보기][[https://youtu.be/ouMp5Gto1]](https://youtu.be/vknkRw1GT1E?si=0iwsJJZAO-zBRKp0)

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

### `NodeServer_A`
node.js express 웹 서버 프레임워크를 사용한 HTTP 서버 입니다. node.js서버에서는 Jwt토큰 발급  및 DB 조회 생성 조회 삭제 등 을 중점으로 처리하기 위해 만든 서브적인 서버 입니다.

---

## 🛠️ ServerCore 주요 구성 요소

### **1. Main**
- **`Types.h`**: 자주 사용하는 타입 및 정의된 별칭 포함.
- **`pch`/`CorePch`**: 프로젝트 전역에서 자주 사용되는 헤더 포함.
- **`CoreMacro`**: 디버깅 및 동기화를 위한 유용한 매크로 정의 (`ASSERT_CRASH`, `사용자 정의 LOCK`, 등).
- **`CoreTLS`** : 각각의 스레드들이 사용할 (id,ticktime,stack 등등) thread local Storage 를 정의
- **`CoreGlobal`**: 전역 객체 정의:
  - **`GThreadManager`**: 스레드를 관리할 Manager클래스 Manager클래스를 통해 외부에서 Thread사용.
  - **`GMemory`**: 메모리 할당 및 해제 관리.
  - **`GSendBufferManager`**: Chunk 방식으로 전송 버퍼 관리. 미리 큰 buffer를 생성하여 필요한 곳에서 일정량 부분을 할당받아 사용.
  - **`GDeadLockProfiler`**: 디버그 모드 일 때 스레드의 function 호출 경로를 스택에 저장하여 DFS(깊이 우선 순위)탐색을 실행해줄 전역 객체
- **`SocketUtils::Init()`**: Windows 소켓 초기화. (connect disconnect accept이벤트 함수 포인터를 미리 받아와야하기 떄문에 전역으로 사용할 객체들과 같이 초기화하면서 미리 이벤트함수들의 함수포인터를 받아옴.)

---

### **2. Memory**
효율적인 메모리 관리 기능:
- **`Allocator`**: 객체에 따른 메모리 할당/해제 정책 정의 
- **`Container.h`**: C++ STL 컨테이너를 커스터마이징 별칭으로 사용.
- **`Memory`**: 전역 `MemoryPool`을 통한 메모리 할당/해제 로직 구현.
- **`MemoryPool`**: 미리 준비된 메모리 공간을 활용하여 빈번한 메모리 할당/해제 최적화.

---

### **3. Thread**
스레드 및 동기화 관련 기능:
- **`Lock`**: Atomic 타입 기반 커스텀 Lock (`WriteLock`, `ReadLock`) 구현.
- **`ThreadManager`**: 스레드들을 관리 할 Manager 객체.
- **`DeadLockProfiler`**:
  - **스레드 로컬 저장소(CoreTLS)**의 `LLockStack` 사용.
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
### **5. Job**
- **`Job`**: Job정의 파일 실행해야 할 함수및 함수실에 필요한 인자를 template으로 받아 job으로 패키징 
- **`JobQueue`**: job을 담아둘 Queue Utils의 LockQueue베이스의 push 및 pop에 lock이 되어 한 스레드만 queue에 진입 하나의 스레드는 Queue안의 job을 처리하고 다른 스레드는 Queue에 push 해주는 형태
- **`JobTimer`**: jobQueue에서 시간 type을 하나 더 받아 우선순위 큐로 시간에 따른 우선순위로 job을 정리할 Queue
- **`GlobalQueue`**: 전역으로 jobqueue를 관리하게 될 Queue
### **5. Utils**
- **`LockQueue`**: 기본  c++ stl queue를 사용하여 push 및 pop 등 queue를 사용할떄 lock을 사용하여 thread safe 부분을 추가한 Queue
  
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
  - **`GameObject`**: Object들의 부모 클래스 (최상위 클래스)
  - **`ObjectManager`**: Object의 생성 및 삭제 또는상태 관리 등을 하는 Manager
  - **`Player`**: GameObject를 상속받은 Player 클래스
  - **`Monster`**: GameObject를 상속받은 Monster 클래스Finite State Machine 사용하요 몬스터의 상태에 따라 다음 행동을 결정 , 유저를 추적하기 위해 a'알고리즘 사용 (참조:https://github.com/ohgeehyun/a-algorithm)
  - **`ProtjectTile`**: GameObject를 상속받은 ProjectTile(투사체) 클래스
  - **`MagicSkill`**: ProjectTile를 MagicSkill 클래스
  - **`Arrow`**: ProjectTile를 상속받은 Arrow 클래스
  #### **5.2 Room  **
  사용자 및 게임 오브젝트가 생성되어 활동하게 될 Room 관련 클래스
  - **`Room`**: 사용자 및 게임 오브젝트가 생성되어 실질적으로 게임에서 일어나는 이벤트를 계산하고 그에 따른 요청을 할 클래스
  - **`RoomManager`**: 여러개의 Room을 관리하게 될 클래스
  - **`MapManager`**:맵의 정보를 저장하고 오브젝트의 위치 관련 정보 및 위치 관련 이벤트를 관리 할 클래스
  #### **5.3 other  **
  그 외 파일
  - **`Server.cpp`**: 프로그램 시작점 (main)
  - **`GameSession`**: ServerCore에서 정의한 session 및 packetsession을 상속받아서 현재 서버에서 필요한 세션을 정의한 클래스
  - **`GameSessionManager`**: GameSession을 관리할 매니저 클래스
  - **`ClientPacketHandler`**: 송수신 해야할 패킷을 처리하기 위한 기능을 정의한 클래스 PacketGenerator에 의해 Protobuf에 패킷 정의시 수정사항이있으면 빌드 시 파일 자동 수정
### **6.Protocol **
   - **`Protocol.proto`**: protobuf pakcet을 정의한 파일
   - **`GenPackets.bat`**: 프로젝트 빌드 시 packetGenerator를 실행시키고 Protobuf관련 파일들을 최신화  시켜줄 batch파일 
   - **`Protocol.pb`**: Protocol.proto 읽어서 생성된 protobuf 헤더와 cpp 파일
### **7.Utils **
  #### **7.1 JwtUtils  **
  - **`JwtUtils`**: Json Web Token 유틸리티 관련 기능 정의 파일
  #### **7.2 RedisUtils  **
  - **`RedisUtils`**: Redis 유틸리티 관련 기능 정의 파일
   #### **7.3 others  **
  - **`JsonUtils`**: json관련 유틸리티 기능 정의 파일
  - **`pch.h`**: GameServer 공통 헤더 및 using 정의 파일
### **8.DB **
   #### **8.1 Mysql **
  - **`MysqlConnection`**: mysql 연결 소켓 클래스
  - **`MysqlConnectionPool`**: mysql 연결 소켓을 관리할 클래스
   #### **8.2 Redis **  
  - **`RedisConnection`**: RedisConnection 연결 소켓 클래스
  - **`RedisConnectionPool`**: RedisConnection 연결 소켓을 관리할 클래스
---
## 🛠️ PacketGenerator 주요 구성 요소
  ### **PacketGenerator**
  - **`PacketGenerator`**: 템플릿을 읽어 ProtoParser에 의해 파싱된 데이터를 사용하여 템플릿을 만들어주는 파일
  - **`ProtoParser`**: .proto파일을 읽어 파싱해 줄 파일
  - **`Templates`**: 템플릿 코드를 모아둔 dir PacketHandler.h 템플릿 위치
---
## 🛠️ NodeServer_A 주요 구성 요소
 ★현재 Node.js서버는 서버는 추가작업 진행중이며 RestFulapi 형식에 맞춰 수정 작업 진행중입니다.★
  ### **Db**
  - **`connection_pool.js`**: mysql 연결  객체 관리  
  - **`User.js`**: mysql 사용자 정보 관련 기능 관리 
  - **`Utils.js`**: mysql 관련 유틸리티 기능 관리 (ex.CURD)등
 ### **jsonwebtoken**
      - **`jwt.js`**: jwt 생성 , 검증, 인증 미들웨어 등 jwt 관련 기능 관리
 ### **Redis** 
  - **`connection_pool.js`**: Redis 연결 객체 관리
  - **`room_chat.js`**: Redis의 채팅 관련 데이터 관리 
  - **`Room.js`**:  Redis의 방 정보 관련 데이터 관리
 ### **routes** 
  - **`chat.js`**: chat 관련 http method 지정 파일
  - **`room.js`**: room 관련 http method 지정 파일
  - **`User.js`**: user 관련 http method 지정 파일
 ### **Utils** 
  - **`InitEnv.js`**: .env에 jwt 에 사용할 secretkey 등 서버 실행시 초기화해야할 설정 파일  


---
## 📝 개발 환경
- **개발툴**:visual studio 2021 , visual studio code
- **언어**: C++ 14, Python 3.9
- **네트워크 모델**: IOCP (I/O Completion Port)
- **프로토콜 자동화**: Google Protocol Buffers
- **클라이언트**: Unity
- **DB(aws 서버)**: mysql , Redis
- **other server**: node.js(express)

---
