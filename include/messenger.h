#ifndef __messenger
#define __messenger

typedef struct {
    char* sender_nick;
    char* message;
} messageinfo;

typedef struct {
    messageinfo** messages; 
    int message_count;
} messagequeue;

messageinfo* new_message(char* sender_nick, char* message_text);
messageinfo* format_as_msg(char* from, char* to, char* message_text);

messagequeue* new_messagequeue();
void messagequeue_push(messagequeue* queue, messageinfo* message);
messageinfo* messagequeue_pop(messagequeue* queue);
void messagequeue_destroy(messagequeue* queue);


#endif
