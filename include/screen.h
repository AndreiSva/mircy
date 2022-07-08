#ifndef __screen
#define __screen

#define ctrl(ch) ((ch) & 0x1f)

#include "../include/mircy.h"
#include "../include/messenger.h"

typedef struct {
    int screen_sizex;
    int screen_sizey;
    
    int cursor_pos;
    int shift;
    int yshift;    

    char* current_message;
    int current_message_length;
} screeninfo;

void mircy_draw_screen(screeninfo* info, messagequeue* messages);
void mircy_screen_eventloop(screeninfo* info, sessioninfo* session_info);

void mircy_init_screen(sessioninfo* current_session_info);

#endif
