#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
//DONE: use winapi to draw coloured text! ✔

#include "console/console.h"
#include <windows.h>
//TODO: Add error checking
//DONE: check for winner! ✔
//EHH: add propmt when there is a winner 〰
const u8 SPRITE_WIDTH = 15;
const u8 SPRITE_HEIGHT = 8;

const u8 BOARD_WIDTH = 3;
const u8 BOARD_HEIGHT = 3;

const u8 MAP_HEIGHT = 3 * 8;



typedef enum {
    X = 1,
    O = 0,
    None = -1
} Player;
typedef enum {
    NoErr = 0,
    OutOfBounds = 1,
    SeatTaken   = 2,


} MoveError;
typedef struct {
    i8 board[9];
    u8 done;
    Player current_player;
    MoveError last_move_was_error;
    ConsolePoint last_valid_move;
    u16 board_state[2]; //lower 9 bits used to represent board
    u16 win;
    ConsolePoint selected;
    u8 plays;
    



} Game;

Game make_move(Game g, ConsolePoint p);

const char* top_row_line = "  --------------""---------------""--------------" "\n";
const char * nothing = "              ";
const char* X_S[] = {
    "   \\\\    //   ",
    "    \\\\  //    ",
    "     \\\\//     ",
    "      \\\\      ",
    "     //\\\\     ",
    "    //  \\\\    ",
    "   //    \\\\   ",
    "              "
};
const char* O_S[] = {
    "    ------    ",
    "  /    _   \\  ",
    " /   /   \\  \\ ",
    "|   |     |  |",
    "|   |     |  |",
    " \\   \\ _ /  / ",
    "  \\        /  ",
    "    ------    "
};

const char** PLAYERS[] = {O_S,X_S};
const char PlayerAsChar[] = {'O','X'};
const ConsoleColourBasic PlayerColours[] = {Blue,Red};
const u16 win_states[] = {
    7        ,// 0 0000 0111
    7 << 3   ,// 0 0011 1000
    7 << 6   ,// 1 1100 0000
    0x49     ,// 0 0100 1001  
    0x49 << 1,// 0 1001 0010
    0x49 << 2,// 1 0010 0100
    0x111    ,// 1 0001 0001
    0x54     ,// 0 0101 0100
};

ConsoleDirection char_to_direction(wchar_t c) {
    switch (c)
    {
    case 'W': case 'K': case VK_UP:
        return UP;
    case 'A': case 'H': case VK_LEFT:
        return LEFT;
    case 'S': case 'J': case VK_DOWN:
        return DOWN;
    case 'D': case 'L': case VK_RIGHT:
        return RIGHT;
    
    default:
        return INVALID_DIR;
    }
}
ConsolePoint dir_to_point(ConsoleDirection dir) {
    switch(dir) {
        case UP:
            return (ConsolePoint) {.x=0,.y=-1};
        case LEFT:
            return (ConsolePoint) {.x=-1,.y=0};
        case DOWN:
            return (ConsolePoint) {.x=0,.y=1};
        case RIGHT:
            return (ConsolePoint) {.x=1,.y=0};
        default:
            return (ConsolePoint) {.x=0,.y=0};
    }
}
void clean_up() {
    clear_screen();
    revert_console_attributes();

}
void show_board(Game* g);
void show_winner_or_stalemate(Game* g) {
    Player p = g->current_player;
    if(p == None) {
        return; // shouldn't happen but you never know
    }
    char text[20];

    char w = PlayerAsChar[1 - g->current_player];

    snprintf(text,20,"%c is the winner!\n",w);

    

    //sprintf_s(text,50,w);
    if(g->plays == 9 && g->win == 0) {
        write_styled_and_coloured_text(L"Stalemate!\n", (ConsoleColour) {.fg=Yellow,.bg=DefaultBackground}, Bold);
    }else {
        write_styled_and_coloured_text(text,(ConsoleColour) {.fg=Green,.bg=DefaultBackground},Bold);
    }
    fflush(stdout);
    
    show_board(g);
    wprintf(L"Press q to quit!\n");
    

}
int max_(int a, int b) {
    return (a >= b) * a + b * (b > a);
}
int min_(int a, int b) {
    return (a <= b) * a + b * (b < a);
}
const uint32_t DEPTH_LIMIT = 1000;

typedef int (*Comparer)(int,int);

const Comparer PLAYER_TYPE[2] = {min_,max_};
const int PLAYER_INITIAL_BEST[] = {2,-2};


int minmax(Game g,int best, Comparer f,  uint32_t depth, int trackers[2]) {
    if(g.win) {
        //printf("From where I'm standing, looks like %c is the winner\n",PlayerAsChar[1 - g.current_player]);
      
        if(g.current_player == X) {
            return -1;
        }else {
            return 1;
        }
    }else if(g.plays >= 9) {
        //printf("From where I'm standing, looks like it's a stalemate\n");
        return 0;
    }
    

    for(int row = 0; row < BOARD_HEIGHT; row++) {
        for(int col = 0; col < BOARD_WIDTH; col++) {
            if((g.board[row * BOARD_WIDTH + col] != None) ){ continue; }
            const Game new_state = make_move(g, (ConsolePoint){.x=col,.y=row});

            int m = minmax(
                new_state,
                PLAYER_INITIAL_BEST[new_state.current_player],
                PLAYER_TYPE[new_state.current_player],
                depth + 1,
                trackers);
            best = f(best,m);
            trackers[new_state.current_player] = f(trackers[new_state.current_player],m);
            if(trackers[0] <= trackers[1]) { // beta <= alpha
                return best;
            }
            
        }
    }
    return best;


}

ConsolePoint next_move(Game g) {
    int best = -2;
    ConsolePoint best_move = {.x = 0, .y = 0};
    for(int row = 0; row < BOARD_HEIGHT; row++) {
        for(int col = 0; col < BOARD_WIDTH; col++) {
            if (g.board[row * BOARD_WIDTH + col] != None) { continue; }
            ConsolePoint p = {.x = col, .y = row};
            Game new_state = make_move(g, p);
                                                        // {alpha, beta}
            int move = minmax(new_state,2,min_, 0,(int[2]){2,-2});

            if(move > best) {
                //printf("Current best move is : %d\n", move);
                best = move;
                best_move = p;

            }

        }
    }

    return best_move;


    

}
u16 check_winner(Game* g) {
    
    u16 winner = 0;      
    for(int i = 0; i < 8; i++) {
        winner = (win_states[i] & g->board_state[g->current_player]) == win_states[i];
        if(winner != 0) {
            return win_states[i];
        }
    }

    return false;  
}
ConsolePoint get_user_point() {
    puts("Please enter the row and column coordinates of where you want to play? (1 <= row <=3) and (1 <= col <= 3)");
    u8 row,col;
    scanf_s("%hhd,%hhd",&row,&col);
    return (ConsolePoint) {.x= col-1,.y=row-1};
}
Game make_move(Game g, ConsolePoint p) {
    MoveError was_error = NoErr;
    if(p.x >= BOARD_WIDTH || p.y >= BOARD_HEIGHT) {
        was_error = OutOfBounds;
        goto error;
    }
    i8* cell = &g.board[p.y * BOARD_WIDTH + p.x];
    if(*cell != None) {
        was_error = SeatTaken; //even though they should  be mutually exclusive events
        goto error;
    }
    *cell = g.current_player;
    g.board_state[g.current_player] |= (1 << (p.y * BOARD_WIDTH + p.x));
    g.plays += 1;
    g.win = check_winner(&g);
    g.done = g.win != 0 || g.plays == 9;

    g.current_player ^= 1;
    g.last_valid_move = p;

error:
    g.last_move_was_error = was_error;

    return g;
}


void show_board(Game* g) {
    
    
    for(int row = 0; row < BOARD_HEIGHT; row++) {
        printf(top_row_line);

        for(int sprite_line = 0; sprite_line < SPRITE_HEIGHT; sprite_line++) {
            for(int col = 0; col < BOARD_WIDTH; col++) {
                printf("|");
                ConsoleColour colour = default_console_colour();
                char const* s = nothing;
                i8 cell = g->board[row * BOARD_WIDTH + col];
                if (cell != None) {
                    s = PLAYERS[cell][sprite_line];
                    colour.fg = PlayerColours[cell];
                }
                if(g->done && 1 << (row * BOARD_WIDTH + col) & g->win) {
                    colour.bg = BGGreen;
                }else if(g->selected.x == col && g->selected.y == row) {
                    colour.bg = BGWhite;
                    if(cell == None) {
                        s = PLAYERS[(int)g->current_player][sprite_line];
                        colour.fg = PlayerColours[g->current_player];
                    }

                }
                write_coloured_text(s,colour);

               

            }
            printf("|\r\n");
        }
        
    }
    puts(top_row_line);
    

}
Game default_game() {
    Game g = {
        .done = false,
        .current_player = X,
        .board = {-1},
        .last_move_was_error=false, 
        .board_state = {0,0},
        .win = 0,
        .selected = (ConsolePoint){.x=0,.y=0},
        .plays = 0};
    for(int i = 0; i < BOARD_HEIGHT * BOARD_WIDTH; i++) {
        g.board[i] = -1;
    }
    return g;
    
}
//X is ai, O is human

int main() {
    prepare_console(ENABLE_VIRTUAL_TERMINAL_PROCESSING,ENABLE_WINDOW_INPUT);
    
    Game tiktok = default_game();
    set_window_title(L"tictactoe");
    hide_cursor();
    while(true) {
        if(tiktok.current_player == X) {
            ConsolePoint ai_move = next_move(tiktok);
            tiktok = make_move(tiktok,ai_move);

            
        }else{ 
            EventList e = get_event_list();
            for(int i = 0; i < e.length; i++) {
                INPUT_RECORD current_event = e.inputRecord[i];
                switch (current_event.EventType)
                {
                case KEY_EVENT:
                    KEY_EVENT_RECORD kevent = current_event.Event.KeyEvent;
                    if(kevent.bKeyDown) {
                        if(kevent.wVirtualKeyCode != 'Q' && kevent.wVirtualKeyCode != VK_RETURN){
                            ConsoleDirection dir = char_to_direction(kevent.wVirtualKeyCode);
                            ConsolePoint p = dir_to_point(dir);
                            tiktok.selected.x = (tiktok.selected.x + p.x) % (i8)BOARD_WIDTH;
                            tiktok.selected.y = (tiktok.selected.y + p.y) % (i8)BOARD_HEIGHT;
                            if(tiktok.selected.x < 0 || tiktok.selected.x >= 3) { // i have no idea why i have to do these checks
                                tiktok.selected.x = 2;
                            }
                            if(tiktok.selected.y < 0 || tiktok.selected.y >= 3) {
                                tiktok.selected.y = 2;
                            }

                        }else if(kevent.wVirtualKeyCode == VK_RETURN) {
                            tiktok = make_move(tiktok,tiktok.selected);
                            
                        }else {
                            clean_up();
                            exit(0);
                        }

                    }
                    break;
                
                default:

                    break;
                }
            }
        }
        
        show_board(&tiktok);
        if(tiktok.done) {
            show_winner_or_stalemate(&tiktok);
            
            tiktok = default_game();
        }
        move_cursor(UP,MAP_HEIGHT + 7);
    }
    
}


