#include "pch.h"
#include "SendBuffer.h"
/*-----------------------
        SendBuffer
-------------------------*/

SendBuffer::SendBuffer(SendBufferChunkRef owner, BYTE* buffer, uint32 allocsize)
    :_owner(owner), _buffer(buffer), _allocSize(allocsize)
{

}

SendBuffer::~SendBuffer()
{

}

void SendBuffer::Close(uint32 writeSize)
{
    ASSERT_CRASH(_allocSize >= writeSize);
    _writeSize = writeSize;
    _owner->Close(_writeSize);
}
/*-----------------------
      SendBufferChunk
-------------------------*/

SendBufferChunk::SendBufferChunk()
{
}

SendBufferChunk::~SendBufferChunk()
{
}

void SendBufferChunk::Reset()
{
    _open = false;
    _usedSize = 0;
}

SendBufferRef SendBufferChunk::Open(uint32 allocSize)
{
    ASSERT_CRASH(allocSize <= SEND_BUFFER_CHUNK_SIZE);
    ASSERT_CRASH(_open == false);

    if (allocSize > FreeSize())
        return nullptr;

    _open = true;
    return ObjectPool<SendBuffer>::MakeShared(shared_from_this(), Buffer(), allocSize);
}

void SendBufferChunk::Close(uint32 writeSize)
{
    ASSERT_CRASH(_open == true);
    _open = false;
    _usedSize += writeSize;
}


/*-----------------------
      SendBufferManager
-------------------------*/

SendBufferRef SendBufferManager::Open(uint32 size)
{
    //LSendBufferChunk가 가지고있는 chunkr가없다면
    if (LSendBufferChunk == nullptr)
    {
        LSendBufferChunk = Pop();
        LSendBufferChunk->Reset(); //WRITE_LOCK;
    }
    ASSERT_CRASH(LSendBufferChunk->IsOpen() == false);

    // 다 썼으면 버리고 교체
    if (LSendBufferChunk->FreeSize() < size)
    {
        LSendBufferChunk = Pop();
        LSendBufferChunk->Reset();
    }

    return LSendBufferChunk->Open(size);
}

SendBufferChunkRef SendBufferManager::Pop()
{
    {
        WRITE_LOCK;
        if (_sendBufferChunks.empty() == false)
        {
            SendBufferChunkRef sendBufferChunk = _sendBufferChunks.back();
            _sendBufferChunks.pop_back();
            return sendBufferChunk;
        }
    }
    //shared ptr로 포인터와 커스텀삭제자를 넣어 다사용되고 삭제될떄 PushGlobal을 호출하여 vector의 끝에 다시 추가
    return SendBufferChunkRef(xnew<SendBufferChunk>(), PushGlobal);
}

void SendBufferManager::Push(SendBufferChunkRef buffer)
{
    WRITE_LOCK;
    _sendBufferChunks.push_back(buffer);
}

void SendBufferManager::PushGlobal(SendBufferChunk* buffer)
{
    GSendBufferManager->Push(SendBufferChunkRef(buffer, PushGlobal));
}