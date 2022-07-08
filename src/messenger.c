#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "../include/messenger.h"

messageinfo* new_message(char* sender_nick, char* message_text) {
    messageinfo* message = malloc(sizeof(messageinfo));
    message->sender_nick = malloc(strlen(sender_nick));
    message->message = malloc(strlen(message_text) + 1);
    strcpy(message->message, message_text);
    strcpy(message->sender_nick, sender_nick);
    return message;
}

messageinfo* format_as_msg(char* from, char* to, char* message_text) {
    char* msg_message = alloca(strlen(from) + strlen(" -> ") + strlen(to));
    sprintf(msg_message, "%s -> %s", from, to);
    messageinfo* message = new_message(msg_message, message_text);
    return message;
}

messagequeue* new_messagequeue() {
    messagequeue* queue = malloc(sizeof(messagequeue));
    queue->message_count = 0;
    queue->messages = malloc(0);
    return queue;
}

void messagequeue_push(messagequeue* queue, messageinfo* message) {
    queue->messages = realloc(queue->messages, sizeof(messageinfo*) * ++queue->message_count);
    queue->messages[queue->message_count - 1] = message;
}

messageinfo* messagequeue_pop(messagequeue* queue) {
    messageinfo* return_message = queue->messages[0];
    for (int i = 0; i <= queue->message_count; i++) {
        queue->messages[i] = queue->messages[i + 1];
    }
    queue->messages = realloc(queue->messages, sizeof(messageinfo*) * --queue->message_count);
    return return_message;
}
