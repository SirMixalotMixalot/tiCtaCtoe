#include <stddef.h>
#include <windows.h>
#include <stdint.h>
#ifndef CONSOLE_LIB 
#define CONSOLE_LIB

#ifndef EVENT_LIST_BUFFER_SIZE
    #define EVENT_LIST_BUFFER_SIZE 20
#endif
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
    int64_t x;
    int64_t y;
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
int prepare_console(uint32_t output_flags, uint32_t input_flags);
int write_styled_and_coloured_text(const wchar_t * text, ConsoleColour colour, STYLE s);
int write_coloured_text(const wchar_t* text, ConsoleColour col);
int write_styled_text(const wchar_t* text, STYLE s);
EventList get_event_list();
int move_cursor(ConsoleDirection d, uint32_t length);

int set_cursor_position(ConsolePoint p); // 0 means no error
void hide_cursor();
void save_cursor_position();
void restore_cursor_last_saved_position();
void scroll_up(uint8_t n);
void clear_screen();
void set_window_title(const wchar_t* title);
void scroll_down(uint8_t n);
void write_text(const wchar_t* t);
void revert_console_attributes();
#endif // CONSOLE_LIB