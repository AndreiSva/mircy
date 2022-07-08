#ifndef __mircy
#define __mircy

#include "../include/messenger.h"

typedef struct {
    char* nick;
    char* host;
    int port;

    int server_socket;

    char* current_channel;

    messagequeue* message_buffer;

    double delay;
    
} sessioninfo;

#endif
