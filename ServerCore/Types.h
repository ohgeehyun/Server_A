#pragma once
#include <cstdint>
#include <mutex>
#include <atomic>

using BYTE = unsigned char;
using int8 = int8_t;
using int16 = int16_t;
using int32 = int32_t;
using int64 = int64_t;
using uint8 = int8_t;
using uint16 = uint16_t;
using uint32 = uint32_t;
using uint64 = uint64_t;

template<typename T>
using Atomic = std::atomic<T>;
using Mutex = std::mutex;
using UnqueLock = std::unique_lock<std::mutex>;
using LockGuard = std::lock_guard<std::mutex>;

using IocpCoreRef = std::shared_ptr<class IocpCore>;
using IocpObjectRef = std::shared_ptr<class IocpObject>;
using SessionRef = std::shared_ptr<class Session>;
using ListenerRef = std::shared_ptr<class Listener>;
using ServerServiceRef = std::shared_ptr<class ServerService>;
using ClientServiceRef = std::shared_ptr<class ClientService>;
using SendBufferRef = std::shared_ptr<class SendBuffer>;
using SendBufferChunkRef = std::shared_ptr<class SendBufferChunk>;
using PacketSessionRef = std::shared_ptr<class PacketSession>;


#define size16(val) static_cast<int16>(sizeof(val))
#define size32(val) static_cast<int32>(sizeof(val))
#define len16(arr)	static_cast<int16>(sizeof(arr)/sizeof(arr[0]))
#define len32(arr)	static_cast<int32>(sizeof(arr)/sizeof(arr[0]))