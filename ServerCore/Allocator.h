#pragma once
/*--------------------------
        Base Allocator
----------------------------*/

using namespace std;;

class BaseAllocator
{
public:
    static void* Alloc(int32 size);
    static void  Release(void* ptr);


private:
};
/*----------------------------
        StompAllocator
------------------------------*/

class StompAllocator
{
    //최소 페이지단위4kb SYSTEM_INFO클래스를사용하여 운영체제에서 사용하는 최소 메모리 관리단위 확인
    enum { PAGE_SIZE = 0x1000 };

public:
    static void* Alloc(int32 size);
    static void  Release(void* ptr);
};
/*----------------------
    PoolAllocator
-------------------------*/
class PoolAllocator
{
    //최소 페이지단위
    enum { PAGE_SIZE = 0x1000 };

public:
    static void* Alloc(int32 size);
    static void  Release(void* ptr);
};

/*----------------------------
        STL Allocator
------------------------------*/
template<typename T>
class StlAllocator {
public:
    using value_type = T;
    StlAllocator() {};

    template<typename Other>
    StlAllocator(const StlAllocator<Other>&) {};

    T* allocate(size_t count)
    {
        const int32 size = static_cast<int32>(count * sizeof(T));
        return static_cast<T*>(PoolAllocator::Alloc(size));

    }

    void deallocate(T* ptr, size_t count)
    {
        PoolAllocator::Release(ptr);
    }

};