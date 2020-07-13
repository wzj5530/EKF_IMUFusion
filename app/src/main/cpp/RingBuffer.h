//
// Created by Kevin.Wang on 2017/04/26.
//

#ifndef SDK_HANDLER_RINGBUFFER_H
#define SDK_HANDLER_RINGBUFFER_H
#include <stdlib.h>
#include <stdio.h>

enum MessType
{
    MsType_Invalid = 0,
    MsType_Create,
    MsType_Destroy,
    MsType_Start,
    MsType_Stop,
    MsType_Resume,
    MsType_Pause,
    MsType_SurfaceView_Created,
    MsType_SurfaceView_Changed,
    MsType_SurfaceView_Destroyed,
    MsType_Touch_Event

};
class Element
{
public:

    Element()
    {
        messageBody = NULL;
        synced = false;
    }
    ~Element()
    {
        if(messageBody)
        {
            free((void*)messageBody);
            messageBody = NULL;
        }

        synced = false;
    }
    void *  messageBody;
    bool    synced;
    int getMessageType()
    {
        if(messageBody)
        {
            return (int)*((int*)messageBody);
        }
        else
        {
            return MsType_Invalid;
        }
    }
    char * getMessageArgs()
    {
        if(messageBody)
        {
            return ((char*)messageBody+ sizeof(int)*2);
        }
        else
        {
            return NULL;
        }
    }
    void clearMessageBody()
    {
        free((void *) messageBody);
        messageBody = NULL;
    }
};

class RingBuffer {
public:



    RingBuffer( int maxMessages );
    ~RingBuffer();



    Element * 	messages;

    volatile int	head;
    volatile int	tail;
    const int 		maxMessages;

    void        PushMessage(int messageType, const char * msg, bool sync);
    Element*    PopMessage();
    bool        IsEmpty();
    bool        IsFull();
    int         Length();
    void        PrintRingBuffer();

};
#endif //SDK_HANDLER_RINGBUFFER_H
