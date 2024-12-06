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
  - 각 스레드에서 독립적으로 관리하며, 전역 **`SendBufferManager`**로 통합 관리.

---

## 📝 개발 환경
- **언어**: C++ 14, Python 3.9
- **네트워크 모델**: IOCP (I/O Completion Port)
- **프로토콜 자동화**: Google Protocol Buffers
- **클라이언트**: Unity

---
