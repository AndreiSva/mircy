#include <curses.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include "../include/screen.h"
#include "../include/mircy.h"
#include "../include/messenger.h"
#include "../include/commands.h"
#include "../include/net.h"
#include "../include/helpers.h"

void mircy_init_screen(sessioninfo* current_session_info) {
    initscr();
    noecho();
    keypad(stdscr, 1);
    nodelay(stdscr, true);
    cbreak();
    
    screeninfo* info = malloc(sizeof(screeninfo));
    info->cursor_pos = 0;
    info->shift = 0;
    info->current_message = malloc(0);
    info->current_message_length = 0;

    info->screen_sizex = getmaxx(stdscr);
    info->screen_sizey = getmaxy(stdscr);

    mircy_screen_eventloop(info, current_session_info);
    free(info->current_message);
    free(info);
    endwin();
}

void mircy_draw_screen(screeninfo* info, messagequeue* messages) {
    erase();
    
    int message_index = messages->message_count - 1 - info->yshift;

    for (int i = info->screen_sizey - 3; i >= 0; i--) {
        if (messages->message_count >= 1 && message_index >= 0) {

            messageinfo* target_message = messages->messages[message_index];
            int display_message_length = strlen(target_message->message) + strlen(target_message->sender_nick) + 2;
            char* display_message = alloca(display_message_length);
            sprintf(display_message, "%s: %s", target_message->sender_nick, target_message->message);
            
            if (display_message_length > info->screen_sizex) {
                i -= display_message_length / info->screen_sizex;
            }
            
            mvprintw(i, 0, display_message);
            message_index--;
        } else {
            break; 
        }
    }

    wmove(stdscr, info->screen_sizey - 2, 0);
    whline(stdscr, '=', info->screen_sizex);
    wmove(stdscr, info->screen_sizey - 1, 0);

    mvaddch(info->screen_sizey - 1, 0, '>');

    // shift view 
    if (info->current_message_length <= info->screen_sizex - 3) {
        info->shift = 0;
    }

    int j;
    j = info->shift;
    
    for (int i = 2; j < info->current_message_length; j++) {
        mvwaddch(stdscr, info->screen_sizey - 1, i++, info->current_message[j]);
    }
    
    wmove(stdscr, info->screen_sizey - 1, info->cursor_pos + 2 - info->shift);
    refresh();
}

void mircy_screen_eventloop(screeninfo* info, sessioninfo* current_session_info) {
    for (;;) {
            usleep(current_session_info->delay);
            mircy_net_recvmsg(current_session_info);
            mircy_draw_screen(info, current_session_info->message_buffer);

            int ch = getch();

            switch (ch) {
                case KEY_RESIZE:
                    info->screen_sizex = getmaxx(stdscr);
                    info->screen_sizey = getmaxy(stdscr);
                    break;
                case KEY_END:
                    info->yshift = 0;
                    break;
                case KEY_HOME:
                    info->yshift = current_session_info->message_buffer->message_count - 1;
                    break;
                case KEY_BACKSPACE:
                    if (info->cursor_pos != 0) {
                        for (int i = info->cursor_pos - 1; i <= info->current_message_length; i++) {
                            info->current_message[i] = info->current_message[i + 1];
                        }
                        info->current_message = realloc(info->current_message, --info->current_message_length);
                        info->cursor_pos--;

                        if (info->shift > 0) {
                            info->shift--; 
                        }
                    }
                    break;
                case KEY_LEFT:
                    if (info->cursor_pos > 0) {
                        info->cursor_pos--;
                        if (info->current_message_length > info->screen_sizex && info->shift > 0) {
                            info->shift--; 
                        }
                    }
                    break;
                case KEY_RIGHT:
                    if (info->cursor_pos < info->current_message_length) {
                        info->cursor_pos++;
                        if (info->current_message_length > info->screen_sizex && info->current_message_length - info->shift >= info->screen_sizex - 2) {
                            int cursor_x, cursor_y;
                            getyx(stdscr, cursor_y, cursor_x);
                            if (cursor_x == info->screen_sizex - 1) {
                                info->shift++; 
                            }
                        }
                    }
                    break;
                case KEY_UP:
                    if (info->yshift < current_session_info->message_buffer->message_count - 1) {
                        info->yshift++;
                    }
                    break;
                case KEY_DOWN:
                    if (info->yshift > 0) {
                        info->yshift--;
                    }
                    break;
                case ctrl('u'):
                    info->current_message_length = 0;
                    info->cursor_pos = 0;
                    free(info->current_message);
                    info->current_message = malloc(0);
                    info->shift = 0;
                    break;
                case '\n':
                    if (info->current_message_length > 0) {
                        info->cursor_pos = 0;
                        
                        // insert null terminator
                        info->current_message[info->current_message_length] = '\0';

                        if (info->current_message[0] == '/') {
                            switch (command_processor(current_session_info, info->current_message + 1)) {
                                case 127:
                                    free(info->current_message);
                                    return; 
                                case 1:
                                    errmsg(current_session_info->message_buffer, "invalid command");
                                default:
                                    break;
                            }
                        } else {
                            messageinfo* message;
                            // check if current channel is a real channel or a PM
                            if (current_session_info->current_channel[0] == '#') {
                                message = new_message(current_session_info->nick, info->current_message);
                            } else {
                                message = format_as_msg(current_session_info->nick, current_session_info->current_channel, info->current_message);
                            }
                            mircy_net_sendChannel(current_session_info, message);                            
                            messagequeue_push(current_session_info->message_buffer, message);
                        }

                        free(info->current_message);
                        info->current_message = malloc(0);
                        info->current_message_length = 0;
                        info->yshift = 0;
                    }
                    break;
                default:
                    if (ch > 31 && ch < 127) {
                        info->current_message = realloc(info->current_message, ++info->current_message_length);

                        // shift bytes, memmove(3) would be a good option to consider for the future TODO:
                        for (int i = info->current_message_length - 1; i >= info->cursor_pos; i--) {
                            info->current_message[i + 1] = info->current_message[i];
                        }

                        info->current_message[info->cursor_pos] = ch;
                        info->cursor_pos++;

                        if (info->current_message_length >= info->screen_sizex - 3) {
                            info->shift++;
                        }
                    }
            }
    }
    
}
