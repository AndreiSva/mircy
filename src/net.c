#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdlib.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/select.h>

#include "../include/net.h"
#include "../include/messenger.h"
#include "../include/helpers.h"

/*
* This is the file that is most likely to cause problems on systems
* other than OpenBSD, I have not tested the portability of the functions
* used here.
*/


FILE* rv;

void mircy_init_net_connection(sessioninfo* info) {
    int server_socket = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in server;
    struct hostent* phost = gethostbyname(info->host);
    
    if (h_errno != NETDB_SUCCESS) {
        if (h_errno == HOST_NOT_FOUND) {
            err("unknown host");
        } else {
            fprintf(stderr, "gethostbyname failed with error code %i", h_errno);  
            err("Aborting");
        }
    }
    
    server.sin_addr.s_addr = inet_addr(inet_ntoa(*(struct in_addr*)phost->h_addr_list[0]));
    server.sin_family = AF_INET;
    server.sin_port = htons(info->port);

    struct timeval timeout;
    timeout.tv_sec = 0;
    timeout.tv_usec = info->delay;
    
    setsockopt(server_socket, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout)); 
    
    connect(server_socket, (struct sockaddr*)&server, sizeof(server));

    fcntl(server_socket, F_SETFL, fcntl(server_socket, F_GETFL) | O_NONBLOCK);
    info->server_socket = server_socket;

    rv = fdopen(info->server_socket, "r");
}

// TODO: Have these functions check for failiures
// also consider making a common function for these to reduce duplicate code
void mircy_net_sendChannel(sessioninfo* info, messageinfo* message) {
    char* sent_message = alloca(strlen("PRIVMSG") + 1 + strlen(info->current_channel) + 2 + strlen(message->message) + 2);
    sprintf(sent_message, "PRIVMSG %s :%s\r\n", info->current_channel, message->message);
    send(info->server_socket, sent_message, strlen(sent_message), 0);
}

void mircy_net_sendRaw(sessioninfo* info, char* str) {
    char* sent_message = alloca(strlen(str) + 2);
    sprintf(sent_message, "%s\r\n", str);
    send(info->server_socket, sent_message, strlen(sent_message), 0);
}

void mircy_net_sendNick(sessioninfo* info, char* str, char* channel) {
    char* sent_message = alloca(strlen("PRIVMSG") + 1 + strlen(channel) + 2 + strlen(str) + 2);
    sprintf(sent_message, "PRIVMSG %s :%s\r\n", channel, str);
    send(info->server_socket, sent_message, strlen(sent_message), 0);
}

void mircy_net_pong(sessioninfo* info) {
    char* sent_message = alloca(strlen("PONG") + strlen(info->host) + 4);
    sprintf(sent_message, "PONG :%s\r\n", info->host);
    send(info->server_socket, sent_message, strlen(sent_message), 0);
}

void mircy_net_nick(sessioninfo* info) {
    char* sent_message = alloca(strlen("NICK") + 1 + strlen(info->nick) + 2);
    sprintf(sent_message, "NICK %s\r\n", info->nick);
    send(info->server_socket, sent_message, strlen(sent_message), 0);
}
void mircy_net_joinChannel(sessioninfo* info, char* channel) {
    char* sent_message = alloca(strlen("JOIN") + 1 + strlen(channel) + 2);
    sprintf(sent_message, "JOIN %s\r\n", channel);
    send(info->server_socket, sent_message, strlen(sent_message), 0);
}

void mircy_net_auth(sessioninfo* info) {
    char* sent_message = alloca(strlen("USER") + 1 + (strlen(info->nick) * 2) + 7 + 2);
    sprintf(sent_message, "USER %s 8 x : %s\r\n", info->nick, info->nick);
    send(info->server_socket, sent_message, strlen(sent_message), 0);
}


messageinfo* mircy_net_processline(sessioninfo* info, char* message) {
    // parses the nick and message content out of the message
    if (message[0] == ':') {
        // copy of original message
        char* msg = alloca(strlen(message));
        strcpy(msg, message);

        char* identity = strtok(message, " ");
        char* nick = alloca(strlen(identity));
        strcpy(nick, identity);
        char* code = strtok(NULL, " ");
        char* type = strtok(NULL, " ");
        char* message_content = strstr(msg, " :") + 2;
        
        if (nick == NULL) {
            nick = alloca(strlen(info->host));
            strcpy(nick, info->host);
        } else if (strcspn(nick, "!") < strlen(nick)) {
            nick[strcspn(nick, "!")] = '\0';
            nick++;
        } else {
            nick++;
        }

        if (!strcmp("005", code)) {
            return NULL;
        }
        // if (message_content == NULL) {
        //     return NULL;
        // } else {
        //     message_content += 2;
        // }
        
        // if (strcspn(nick, "\r\n") > strlen(nick)) {
        // something is going wrong in the line below
        // }
        message_content[strcspn(message_content, "\r\n")] = '\0';
        
        messageinfo* ret;
        ret = new_message(nick, message_content);

        // we are being private messaged
        // char* msgstring = alloca(strlen("PRIVMSG ") + strlen(info->nick));
        // sprintf(msgstring, "PRIVMSG %s", info->nick);
        // if (strstr(msg, msgstring) != NULL) {
        //     ret = format_as_msg(nick, info->nick, message_content);
        // } else {
        //     ret = new_message(nick, message_content);
        //}
        messagequeue_push(info->message_buffer, ret);
        return ret;
    } else if (message[0] == 'P') {
        mircy_net_pong(info);
        return NULL;
    } 
    return NULL;
}

void mircy_net_recvmsg(sessioninfo* info) {
    // recieve a message via irc

    int count;
    ioctl(info->server_socket, FIONREAD, &count);
    // don't attempt to recieve a message unless we know there is new data to read
    if (count > 0) {
        char* message = NULL;
        size_t len = 0;
        ssize_t read;
        setvbuf(rv, message, _IOLBF, 1024);
        while ((read = getline(&message, &len, rv)) != -1) {
            mircy_net_processline(info, message);
        }
    }
 }
