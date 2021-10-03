#include <windows.h>
#include <wchar.h>
#include <stdio.h>
#include "console.h"

#define ESC L"\x1b"

struct {
    HANDLE h_out;
    HANDLE h_in;
    
} console;

ConsoleColour default_console_colour() {
    return (ConsoleColour) {.red = 0,.green = 0,.blue = 0,.fg = DefaultForeground,.bg = DefaultBackground};
}

HANDLE prepare_std_handle(DWORD handletype, ULONG flags) {
    HANDLE h = GetStdHandle(handletype);
    if(h == INVALID_HANDLE_VALUE) exit( GetLastError());

    DWORD mode = 0;
    if(GetConsoleMode(h,&mode) == 0) exit ( GetLastError() );

    mode |= flags;
    if(SetConsoleMode(h,mode) == 0) exit ( GetLastError() );

    return h;
}
int prepare_console(uint64_t oflags, uint32_t iflags) {
    console.h_out = prepare_std_handle(STD_OUTPUT_HANDLE, oflags);
    console.h_in = prepare_std_handle(STD_INPUT_HANDLE,iflags);
    return 0;

}
EventList get_event_list() {
    EventList evl;
    if(ReadConsoleInput(
        console.h_in,
        evl.inputRecord,
        EVENT_LIST_BUFFER_SIZE,
        &evl.length
    ) == 0) exit(GetLastError());
    return evl;
    
}
int write_styled_and_coloured_text(const wchar_t * text, ConsoleColour colour, STYLE s) {

    if (colour.fg == NoSimpleColour) {
        wprintf(L"\x1b[38;2;%d;%d;%d;m",colour.red,colour.green,colour.blue);
    }else {
        wprintf(L"\x1b[%d;%dm",colour.fg,colour.bg);
    }
    if(s != NoStyle) {
        wprintf(L"\x1b[%dm",s);
    }
    printf("%s",text);
    //reset terminal attributes to default
    wprintf(L"\x1b[0m");
    
    return 0;
}
int write_coloured_text(const wchar_t* text, ConsoleColour col) {
    return write_styled_and_coloured_text(text,col,NoStyle);
}
int write_styled_text(const wchar_t* text, STYLE s) {
    return write_styled_and_coloured_text(text,default_console_colour(),s);
}

int move_cursor(ConsoleDirection d, uint32_t length) {
    char dir = '\0';
    switch (d)
    {
    case LEFT:
        dir = 'D';
        break;
    case RIGHT:
        dir = 'C';
        break;
    case DOWN:
        dir = 'B';
        break;
    case UP:
        dir = 'A';
        break;
    default:
        return -1; // Invalid direction
        break;
    }
    wprintf(L"\x1b[%u%c",length,dir);
    return 0;
}
void save_cursor_position() {
    wprintf_s(L"\x1b7");
}
void restore_cursor_last_saved_position() {
    wprintf_s(L"\x1b8");
}

int set_cursor_position(ConsolePoint p) {
    wprintf(L"\x1b[%d;%dH",p.y,p.x);
    return 0;
}
void hide_cursor() {
    wprintf(L"\x1b[?25l");
}
void scroll_up(uint8_t n) {
    wprintf(L"\x1b[%dS",n);
}
void scroll_down(uint8_t n) {
    wprintf(L"\x1b[%dT",n);
}
void clear_screen() {
    wprintf(L"\x1b[2J");
}
void set_window_title(const wchar_t* title) {
    wprintf(L"\x1b]2;%ls\x07",title);
}
void write_text(const wchar_t* text) {
    write_styled_and_coloured_text(text,default_console_colour(),NoStyle);
}
void revert_console_attributes() {
    wprintf(L"\x1b[!p");
}