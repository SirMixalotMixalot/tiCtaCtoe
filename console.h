#include <stddef.h>
#include <windows.h>
#ifndef CONSOLE_LIB 
#define CONSOLE_LIB
typedef unsigned char u8;
typedef unsigned long u64;
typedef signed long i64;
typedef signed char i8;
typedef unsigned short u16;

#define EVENT_LIST_BUFFER_SIZE 20
typedef struct {
    INPUT_RECORD inputRecord[EVENT_LIST_BUFFER_SIZE];
    int length;
} EventList;
typedef enum {
    Bold = 1,
    Underline = 4,
    Negative = 7,
    NoStyle = 0

}STYLE;
typedef struct {
    u64 x;
    u64 y;
} ConsolePoint;
typedef enum {
    UP,
    DOWN,
    LEFT, 
    RIGHT,
    INVALID_DIR,
}ConsoleDirection;
typedef enum {
    NoSimpleColour = 0,
    Black = 30,
    Red,
    Green,
    Yellow,
    Blue,
    Magenta,
    Cyan,
    White,
    DefaultForeground = 39,
    BGGreen = 42,
    BGWhite = 47,
    DefaultBackground = 49,
} ConsoleColourBasic;
typedef struct  {
    int red;
    int green;
    int blue;
    ConsoleColourBasic fg;
    ConsoleColourBasic bg;
} ConsoleColour;
ConsoleColour default_console_colour();
int prepare_console(unsigned short output_flags, unsigned short input_flags);
int write_styled_and_coloured_text(const wchar_t * text, ConsoleColour colour, STYLE s);
int write_coloured_text(const wchar_t* text, ConsoleColour col);
int write_styled_text(const wchar_t* text, STYLE s);
EventList get_event_list();
int move_cursor(ConsoleDirection d, unsigned int length);

int set_cursor_position(ConsolePoint p); // 0 means no error
void hide_cursor();
void save_cursor_position();
void restore_cursor_last_saved_position();
void scroll_up(u8 n);
void clear_screen();
void set_window_title(const char* title);
void scroll_down(u8 n);
void write_text(const char* t);
#endif // CONSOLE_LIB