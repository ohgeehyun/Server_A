#pragma once
#include "CorePch.h"

enum
{
    SLIST_ALIGNMENT = 16,
};

/*------------------------------------
              MemoryHeader
단일 연결 리스트(SLIST_ENTRY)를 상속받아 사용
16바이트로 정렬해서 사용시 cpu사용율에 좋다.
-------------------------------------*/
DECLSPEC_ALIGN(SLIST_ALIGNMENT)
struct MemoryHeader : public SLIST_ENTRY
{
    //[MemoryHeader][Data]
    MemoryHeader(int32 size) : allocSize(size) {};

    static void* AttachHeader(MemoryHeader* header, int32 size)
    {
        new(header)MemoryHeader(size);//placement new
        return reinterpret_cast<void*>(++header);
    }
    static MemoryHeader* DetacchHeader(void* ptr)
    {
        MemoryHeader* header = reinterpret_cast<MemoryHeader*>(ptr) - 1;
        return header;
    }

    int32 allocSize;
};


/*------------------------------------
              MemoryPool
-------------------------------------*/
DECLSPEC_ALIGN(SLIST_ALIGNMENT)
class MemoryPool
{
public:
    MemoryPool(int32 allocSize);
    ~MemoryPool();

    void Push(MemoryHeader* ptr);
    MemoryHeader* Pop();

private:
    SLIST_HEADER _header;
    int32 _allocsize = 0;
    atomic<int32> _useCount = 0;//메모리의 갯수 Pool의 카운터아님.
    atomic<int32> _reserveCount = 0;

};