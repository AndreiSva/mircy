#include <string.h>
#include <stdlib.h>

#include <stdio.h>
#include "../include/commands.h"
#include "../include/net.h"
#include "../include/helpers.h"
#include "../include/messenger.h"

char command_processor(sessioninfo* info, char* command) {
    // Do nothing if empty command
    if (strlen(command) == 0) {
        return 0;
    }
    char* original_command = alloca(strlen(command));
    strcpy(original_command, command);
    char* arg = strtok(command, " ");
    if (!strcmp("set", command)) {
        // change settings
    } else if (!strcmp("quit", command)) {
        return 127; 
    } else if (!strcmp("nick", arg)) {
        char* new_nick = strtok(NULL, " ");
        if (new_nick != NULL) {
            info->nick = malloc(sizeof(new_nick));
            strcpy(info->nick, new_nick);
        }
        mircy_net_nick(info);
    } else if (!strcmp("clear", command)) {
        for (int i = 0; i < info->message_buffer->message_count; i++) {
            free(info->message_buffer->messages[i]->sender_nick);
            free(info->message_buffer->messages[i]->message);
            free(info->message_buffer->messages[i]); 
        }
        info->message_buffer->message_count = 0;
    } else if (!strncmp("msg", command, 3)) {
        char* nick = strtok(NULL, " ");
        if (nick != NULL) {
            char* mesg = strstr(original_command, nick) + strlen(nick) + 1;
            mircy_net_sendNick(info, mesg, nick);
            messagequeue_push(info->message_buffer, format_as_msg(info->nick, nick, mesg));
        }
    } else if (!strncmp("quote", command, 5)) {
        mircy_net_sendRaw(info, original_command + 6); 
    } else {
        return 1;
    }
    return 0;
}
