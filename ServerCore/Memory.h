#pragma once
#include "Allocator.h"

class MemoryPool;
/*----------------
     Memory
-----------------*/
class Memory
{
    enum
    {
        //~1024까지 32단위, 2048까지 128단위, 4095까지 256 단위
        POOL_COUNT = (1024 / 32) + (1024 / 128) + (2048 / 256),
        MAX_ALLOC_SIZE = 4096
    };
public:

    Memory();
    ~Memory();

    void* Allocate(int32 size);
    void Release(void* ptr);

private:
    vector<MemoryPool*>_pools;
    //메모리 크기 <-> 메모리 풀 둘의 크기는 같다 거울? 생각
    //0(1)의 복잡도를 위해 테이블
    MemoryPool* _poolTable[MAX_ALLOC_SIZE + 1];

};

template<typename Type, typename ...Args>
Type* xnew(Args&&...args)
{
    Type* memory = static_cast<Type*>(PoolAllocator::Alloc(sizeof(Type)));
    new(memory)Type(forward<Args>(args)...);

    return memory;
}

template<typename Type>
void xdelete(Type* obj)
{
    obj->~Type();
    PoolAllocator::Release(obj);
}


template<typename Type, typename...Args>
shared_ptr<Type> Make_Shared(Args&&... args)
{
    return shared_ptr<Type>{xnew<Type>(std::forward<Args>(args)...), xdelete<Type>};
}