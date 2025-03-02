#pragma once

template<typename T>
class LockQueue
{
public:
    void Push(T item)
    {
        WRITE_LOCK
        _items.push(item);
    }

    T Pop()
    {
       WRITE_LOCK
        if (_items.empty())
            return T();

       T ret = _items.front();
       _items.pop();
        return ret;
    }

    void PopAll(OUT Vector<T>& items)
    {
        WRITE_LOCK
        //컴파일러나 타입에 따라 while조건 boolean context가 다르게 작동할 가능성 있음.
        while (T item = Pop())
        items.push_back(item);
    }

    void  Clear()
    {
        WRITE_LOCK;
        _items = Queue<T>();
    }

private:
    USE_LOCK;
    Queue<T> _items;
};