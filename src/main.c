#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>

#include "../include/helpers.h"
#include "../include/mircy.h"
#include "../include/screen.h"
#include "../include/net.h"

#include <curses.h>

#ifndef VERSION
#define VERSION "cvs"
#endif

void usage() {
    fprintf(stderr, "usage: mircy [-hv] [-d delay] [-n nick] [-c channel] [-s host] [-p port]\n");
    exit(1);
}

int main(int argc, char** argv) {
    sessioninfo* current_session = malloc(sizeof(sessioninfo));
    current_session->nick = malloc(9);
    current_session->port = 6667;
    current_session->host = calloc(256, 1);
    current_session->message_buffer = new_messagequeue();
    current_session->delay= 0.01 * 1000000;
    int ch;
    while ((ch = getopt(argc, argv, "hs:n:vd:p:c:")) != - 1) {
        switch (ch) {
            case 'v':
                printf("%s\n", VERSION);
                exit(0);
                break;
            case 'n':
                strcpy(current_session->nick, optarg);
                break;
            case 's':
                strcpy(current_session->host, optarg);
                break;
            case 'd':
                ;
                double user_delay = atof(optarg);
                if (user_delay < 0) {
                    err("delay cannot be a negative value"); 
                }
                // delay in microseconds
                current_session->delay = user_delay * 1000000;
                break;
            case 'p':
                current_session->port = atoi(optarg);
                break;                
            case 'c':
                current_session->current_channel = malloc(strlen(optarg));
                strcpy(current_session->current_channel, optarg);
                break;
            case 'h':
            default:
                usage();
                break;
        }
    }

    if (!strcmp(current_session->nick, "")) {
        current_session->nick = getlogin();
    }

    if (!strcmp(current_session->host, "")) {
        err("host (-s) argument required");
    }
    
    if (current_session->current_channel == NULL) {
        err("channel (-c) argument required");
    }
        
    mircy_init_net_connection(current_session);
    mircy_net_nick(current_session);
    mircy_net_auth(current_session);
    // only join if channel is a real channel and not a private message
    if (current_session->current_channel[0] == '#') {
        sleep(2);
        mircy_net_joinChannel(current_session, current_session->current_channel);
    }

    mircy_net_pong(current_session);
    mircy_init_screen(current_session); 
    close(current_session->server_socket);
    free(current_session->host);
    free(current_session->current_channel);
    free(current_session);
    return 0;
}
