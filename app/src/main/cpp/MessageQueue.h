//
// Created by win7 on 2017/4/20.
//

#ifndef SDK_HANDLER_MESSAGEQUEUEOC_H
#define SDK_HANDLER_MESSAGEQUEUEOC_H

#include <pthread.h>
#include "RingBuffer.h"


// This is a multiple-producer, single-consumer message queue.

    class MessageQueue
    {
    public:
        MessageQueue( int maxMessages );
        ~MessageQueue();

        // Shut down the message queue once messages are no longer polled
        // to avoid overflowing the queue on message spam.
        void			Shutdown();
        // Thread safe, callable by any thread.
        // The msg text is copied off before return, the caller can free
        // the buffer.
        // The app will abort() with a dump of all messages if the message
        // buffer overflows.
        void			PostString( int messageType,const char * msg );
        // Builds a printf string and sends it as a message.
        void			PostPrintf( int messageType,const char * fmt, ... );
        // Same as above but these return false if the queue is full instead of an abort.
        bool			TryPostString( int messageType,const char * msg );
        bool			TryPostPrintf( int messageType,const char * fmt, ... );
        // Same as above but these wait until the message has been processed.
        void			SendString( int messageType,const char * msg );
        void			SendPrintf( int messageType,const char * fmt, ... );
        // Returns the number slots available for new messages.
        //int				SpaceAvailable()  { return maxMessages - messageBuffer.Length(); }
        // The other methods are NOT thread safe, and should only be
        // called by the thread that owns the MessageQueue.

        // Returns NULL if there are no more messages, otherwise returns
        // a string that the caller is now responsible for freeing.
        Element * 	GetNextMessage();
        // Returns immediately if there is already a message in the queue.
        void			SleepUntilMessage();
        // Dumps all unread messages
        //void			ClearMessages();
    private:
        // If set true, print all message sends and gets to the log
        static bool		debug;
        bool			shutdown;
//        const int 		maxMessages;
        RingBuffer      messageBuffer;
        bool			synced;
        pthread_mutex_t	mutex;
        pthread_cond_t	posted;
        pthread_cond_t	received;

        bool PostMessage( int messageType, const char * msg, bool sync, bool abortIfFull );
    };



#endif //SDK_HANDLER_MESSAGEQUEUE_H
