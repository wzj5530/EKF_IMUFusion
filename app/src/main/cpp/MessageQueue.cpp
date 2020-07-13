#include "MessageQueue.h"
#include "LogUtils.h"
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

#include <sys/time.h>
#include <sys/resource.h>

bool MessageQueue::debug = false;

MessageQueue::MessageQueue( int maxMessages_ ) :
        shutdown( false ),
        synced( false ),
        messageBuffer(maxMessages_)
{

    pthread_mutexattr_t	attr;
    pthread_mutexattr_init( &attr );
    pthread_mutexattr_settype( &attr, PTHREAD_MUTEX_ERRORCHECK );
    pthread_mutex_init( &mutex, &attr );
    pthread_mutexattr_destroy( &attr );
    pthread_cond_init( &posted, NULL );
    pthread_cond_init( &received, NULL );
}

MessageQueue::~MessageQueue()
{
    pthread_mutex_destroy( &mutex );
    pthread_cond_destroy( &posted );
    pthread_cond_destroy( &received );
}

void MessageQueue::Shutdown()
{
    LOG( "%p:MessageQueue shutdown", this );
    shutdown = true;
}

// Thread safe, callable by any thread.
// The msg text is copied off before return, the caller can free
// the buffer.
// The app will abort() with a dump of all messages if the message
// buffer overflows.
bool MessageQueue::PostMessage( int messageType, const char * msg, bool sync, bool abortIfFull )
{
    if ( shutdown )
    {
        LOG( "%p:PostMessage( %s ) to shutdown queue", this, msg );
        return false;
    }
    if ( debug )
    {
        LOG( "%p:PostMessage( %s )", this, msg );
    }

    pthread_mutex_lock( &mutex );
    if ( messageBuffer.IsFull() )
    {
        pthread_mutex_unlock( &mutex );
        if ( abortIfFull )
        {
            LOG( "MessageQueue overflow" );
            messageBuffer.PrintRingBuffer();
            LOG( "Message buffer overflowed" );
        }
        return false;
    }
    LOG( "PostMessage: UER_EVENT  %d", messageType);
    messageBuffer.PushMessage(messageType,msg,sync);
    pthread_cond_signal( &posted );
    if ( sync )
    {
        pthread_cond_wait( &received, &mutex );
    }
    pthread_mutex_unlock( &mutex );

    return true;
}

void MessageQueue::PostString( int messageType, const char * msg )
{
    PostMessage( messageType, msg, false, true );
}

void MessageQueue::PostPrintf( int messageType, const char * fmt, ... )
{
    char bigBuffer[4096];
    va_list	args;
    va_start( args, fmt );
    vsnprintf( bigBuffer, sizeof( bigBuffer ), fmt, args );
    va_end( args );
    PostMessage( messageType, bigBuffer, false, true );
}

bool MessageQueue::TryPostString(int messageType, const char * msg )
{
    return PostMessage(messageType, msg, false, false );
}

bool MessageQueue::TryPostPrintf(int messageType, const char * fmt, ... )
{
    char bigBuffer[4096];
    va_list	args;
    va_start( args, fmt );
    vsnprintf( bigBuffer, sizeof( bigBuffer ), fmt, args );
    va_end( args );
    return PostMessage(messageType, bigBuffer, false, false );
}

void MessageQueue::SendString(int messageType, const char * msg )
{
    PostMessage(messageType, msg, true, true );
}

void MessageQueue::SendPrintf(int messageType, const char * fmt, ... )
{
    char bigBuffer[4096];
    va_list	args;
    va_start( args, fmt );
    vsnprintf( bigBuffer, sizeof( bigBuffer ), fmt, args );
    va_end( args );
    LOG( "SendPrintf: UER_EVENT  %d", messageType);
    PostMessage(messageType, bigBuffer, true, true );
}
// Returns false if there are no more messages, otherwise returns
// a string that the caller must free.
Element * MessageQueue::GetNextMessage()
{
    if ( synced )
    {
        pthread_cond_signal( &received );
        synced = false;
    }

    pthread_mutex_lock( &mutex );
    if ( messageBuffer.IsEmpty() )
    {
        pthread_mutex_unlock( &mutex );
        return NULL;
    }

    Element * ele = messageBuffer.PopMessage();
    synced = ele->synced;
    pthread_mutex_unlock( &mutex );

    if ( debug )
    {
        LOG( "%p:GetNextMessage() : %s", this, "xxx" );
    }

    return ele;
}

// Returns immediately if there is already a message in the queue.
void MessageQueue::SleepUntilMessage()
{
    if ( synced )
    {
        pthread_cond_signal( &received );
        synced = false;
    }

    pthread_mutex_lock( &mutex );
    if ( !messageBuffer.IsEmpty() )
    {
        pthread_mutex_unlock( &mutex );
        return;
    }

    if ( debug )
    {
        LOG( "%p:SleepUntilMessage() : sleep", this );
    }

    pthread_cond_wait( &posted, &mutex );
    pthread_mutex_unlock( &mutex );

    if ( debug )
    {
        LOG( "%p:SleepUntilMessage() : awoke", this );
    }
}



