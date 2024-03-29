#ifndef __net
#define __net

#include <sys/socket.h>

#include "../include/mircy.h"
#include "../include/messenger.h"

void mircy_net_pong(sessioninfo* info);
void mircy_init_net_connection(sessioninfo* info);
void mircy_net_sendChannel(sessioninfo* info, messageinfo* message);
void mircy_net_sendNick(sessioninfo* info, char* str, char* channel);
void mircy_net_sendRaw(sessioninfo* info, char* str);
void mircy_net_auth(sessioninfo* info);
void mircy_net_nick(sessioninfo* info);
void mircy_net_joinChannel(sessioninfo* info, char* channel);
void mircy_net_recvmsg(sessioninfo* info);
void mircy_net_disconnect();
messageinfo* mircy_net_processline(sessioninfo* info, char* message);

#endif
