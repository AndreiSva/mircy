#ifndef __helpers
#define __helpers

#include "../include/screen.h"
#include "../include/messenger.h"

#define err(str) \
    fprintf(stderr, "ERR: %s\n", str); exit(1)

// ncurses error message to show in the message buffer
#define errmsg(message_buffer, str) \
    messagequeue_push(message_buffer, new_message("ERROR", str))

#define VERSION "v0.01"

#endif
