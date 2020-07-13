//
// Created by Kevin.Wang on 2017/04/26.
//

#include "RingBuffer.h"
#include "LogUtils.h"

#include <assert.h>
#include <stdlib.h>
#include <stdio.h>

RingBuffer::RingBuffer( int maxMessages_ ) :
        maxMessages( maxMessages_ ),
        messages( new Element[ maxMessages_ ] ),
        head( 0 ),
        tail( 0 )
{
    assert( maxMessages > 0 );
}

RingBuffer::~RingBuffer()
{
    delete[] messages;
}


void    RingBuffer::PushMessage(int messageType, const char * msg, bool sync)
{
    const int index = tail % maxMessages;
    messages[index].messageBody = calloc(1,sizeof(int)*2 + strlen(msg) + 1);
    char * messPtr = (char *)messages[index].messageBody;
    *((int *)messPtr) = (int)messageType;
    *((int *)(messPtr + sizeof(int))) = strlen(msg);
    memcpy((void*)(messPtr+sizeof(int)*2),(void*)msg,strlen(msg));
    messages[index].synced = sync;
    tail++;
}
Element*    RingBuffer::PopMessage()
{
    const int index = head % maxMessages;
    head++;
    return &messages[index];

}
bool    RingBuffer::IsEmpty()
{
    return  tail <= head;
}
bool    RingBuffer::IsFull()
{
    return (tail - head >= maxMessages);
}
int     RingBuffer::Length()
{
    return (tail - head);
}
void    RingBuffer::PrintRingBuffer()
{
    for ( int i = head; i < tail; i++ )
    {
        LOG( "%d", messages[i % maxMessages].getMessageType() );
    }
}

