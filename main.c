#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "console/console.h"
#include <windows.h>



// -- TYPEDEFS -- 
typedef int (*Comparer)(int,int);
// -------------

// ----- UTIL FUNCS -----
int max_(int a, int b) {
    return (a >= b) * a + b * (b > a);
}
int min_(int a, int b) {
    return (a <= b) * a + b * (b < a);
}
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
// ----------------------------------


// -- CONSTS -- //
const uint8_t SPRITE_WIDTH = 15;
const uint8_t SPRITE_HEIGHT = 8;

const uint8_t BOARD_WIDTH = 3;
const uint8_t BOARD_HEIGHT = 3;

const uint8_t MAP_HEIGHT = 4 * 8;

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
const char PLAYER_AS_CHAR[] = {'O','X'};
const ConsoleColourBasic PlayerColours[] = {Blue,Red};
const uint16_t win_states[] = {
    7        ,// 0 0000 0111
    7 << 3   ,// 0 0011 1000
    7 << 6   ,// 1 1100 0000
    0x49     ,// 0 0100 1001  
    0x49 << 1,// 0 1001 0010
    0x49 << 2,// 1 0010 0100
    0x111    ,// 1 0001 0001
    0x54     ,// 0 0101 0100
};
const Comparer PLAYER_TYPE[2] = {min_,max_};
const int PLAYER_INITIAL_BEST[] = {2,-2};
//const int* ALPHA_BETAS[] = {(int[]){-2,2},(int[]){2,-2}};
// ----------
// ---Structs ---//
typedef enum {
    X = 1,
    O = 0,
    None = -1
} Player;

typedef struct {
    int8_t board[9];
    uint8_t done;
    Player current_player;
    uint16_t board_state[2]; //lower 9 bits used to represent board
    uint16_t win;
    ConsolePoint selected;
    uint8_t plays;
    Player ai;
} Game;
// --------------------

// -- FUNCTION DECLARATIONS --
Game make_move(Game g, ConsolePoint p);
void show_board(const Game* g);
ConsoleDirection char_to_direction(wchar_t c);
ConsolePoint dir_to_point(ConsoleDirection dir);
void clean_up();
void show_winner_or_stalemate(const Game* g);
int minmax(const Game* g,int best, Comparer f,  uint32_t depth, int trackers[2]);
ConsolePoint next_move(const Game* g);
uint16_t check_winner(const Game* g);
Game default_game();
//  -----------------------

int main() {
    prepare_console(ENABLE_VIRTUAL_TERMINAL_PROCESSING,ENABLE_WINDOW_INPUT);
    
    Game tiktok = default_game();
    set_window_title(L"tictactoe");
    hide_cursor();
    while(true) {
        if(tiktok.current_player == tiktok.ai) {
            ConsolePoint ai_move;
            if(tiktok.plays == 0) { //make a random move if the ai is first
                ai_move = (ConsolePoint){.x = 2, .y = 0};
            }else {
                ai_move = next_move(&tiktok);
            }
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
                            tiktok.selected.x = (tiktok.selected.x + p.x) % (int8_t)BOARD_WIDTH;
                            tiktok.selected.y = (tiktok.selected.y + p.y) % (int8_t)BOARD_HEIGHT;
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
                }
            }
        }
        
        show_board(&tiktok);
        if(tiktok.done) {
            show_winner_or_stalemate(&tiktok);
            
            tiktok = default_game();
        }
        move_cursor(UP,MAP_HEIGHT);
    }
    
}

Game default_game() {
    Game g = {
        .done = false,
        .current_player = rand() % 2,
        .board_state = {0,0},
        .win = 0,
        .selected = (ConsolePoint){.x=0,.y=0},
        .plays = 0,
        .ai = X
        };
    for(int i = 0; i < BOARD_HEIGHT * BOARD_WIDTH; i++) {
        g.board[i] = -1;
    }
    return g;
    
}
ConsolePoint next_move(const Game* g) {
    int best = PLAYER_INITIAL_BEST[g->ai];
    int trackers[2] = {2,-2};
    ConsolePoint best_move = {.x = 0, .y = 0};
    for(int row = 0; row < BOARD_HEIGHT; row++) {
        for(int col = 0; col < BOARD_WIDTH; col++) {
            if (g->board[row * BOARD_WIDTH + col] != None) {
                 continue; 
            }
            ConsolePoint p = {.x = col, .y = row};
            Game new_state = make_move(*g, p);


                                                   
            int move = minmax(&new_state,
                              PLAYER_INITIAL_BEST[new_state.current_player],
                              PLAYER_TYPE[new_state.current_player],
                              0,
                                //{beta,alpha}
                              trackers);
            //printf("Trackers : [%d,%d]",trackers[0],trackers[1]);

            if(PLAYER_TYPE[g->ai](best,move) != best) { 
                //printf("Current best move is : %d\n", move);
                best = move;
                best_move = p;

            }



        }
    }
    return best_move;
}
int minmax(Game* const g, int best, Comparer f, uint32_t depth, int trackers[2]) {
    if (g->win) {
        if (g->current_player == X) { 
            return -1;
        }
        else {
            return 1;
        }
    }
    else if (g->plays >= 9) {
        return 0;
    }
    for (int row = 0; row < BOARD_HEIGHT; row++) {
        for (int col = 0; col < BOARD_WIDTH; col++) {
            if ((g->board[row * BOARD_WIDTH + col] != None)) {
                continue;
            }
            const Game new_state = make_move(*g, (ConsolePoint) { .x = col, .y = row });

            int m = minmax(
                &new_state,
                PLAYER_INITIAL_BEST[new_state.current_player],
                PLAYER_TYPE[new_state.current_player],
                depth + 1,
                trackers);
            best = f(best, m);
            trackers[new_state.current_player] = f(trackers[new_state.current_player], m);
            if (trackers[0] <= trackers[1]) { // beta <= alpha
                return best;
            }

        }
    }
    return best;


}


Game make_move(Game g, ConsolePoint p) {
    uint8_t index = p.y * BOARD_WIDTH + p.x;
    if(p.x >= BOARD_WIDTH || p.y >= BOARD_HEIGHT || g.board[index] != None) {
        return g; 
    }
    g.board[index] = g.current_player;
    g.board_state[g.current_player] |= 1 << index;
    g.plays += 1;
    g.win = check_winner(&g);
    g.done = g.win != 0 || g.plays == 9;

    g.current_player ^= 1;

    return g;
}
void show_board(const Game* g) {
    
    
    for(int row = 0; row < BOARD_HEIGHT; row++) {
        printf(top_row_line);

        for(int sprite_line = 0; sprite_line < SPRITE_HEIGHT; sprite_line++) {
            for(int col = 0; col < BOARD_WIDTH; col++) {
                printf("|");
                ConsoleColour colour = default_console_colour();
                char const* s = nothing;
                int8_t cell = g->board[row * BOARD_WIDTH + col];
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



void show_winner_or_stalemate(const Game* g) {
    Player p = g->current_player;
    if(p == None) {
        return; // shouldn't happen but you never know
    }
    char text[20];

    char w = PLAYER_AS_CHAR[1 - p]; //other player must have won

    snprintf(text,20,"%c is the winner!\n",w);

    

    if(g->plays == 9 && g->win == 0) {
        write_styled_and_coloured_text(L"Stalemate!\n", (ConsoleColour) {.fg=Yellow,.bg=DefaultBackground}, Bold);
    }else {
        write_styled_and_coloured_text(text,(ConsoleColour) {.fg=Green,.bg=DefaultBackground},Bold);
    }
    fflush(stdout);
    
    show_board(g);
    wprintf(L"Press q to quit!\n");
    

}

void clean_up() {
    clear_screen();
    revert_console_attributes();

}





uint16_t check_winner(const Game* g) {
    
    uint16_t winner = 0;      
    for(int i = 0; i < 8; i++) {
        winner = (win_states[i] & g->board_state[g->current_player]) == win_states[i];
        if(winner != 0) {
            return win_states[i];
        }
    }

    return 0;  
}
