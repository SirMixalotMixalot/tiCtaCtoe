#include "console/console.h"
const char* dir_to_str(ConsoleDirection d) {
    switch (d)
    {
    case UP:
        return "UP";
    case DOWN:
        return "DOWN";
    case LEFT:
        return "LEFT";
    case RIGHT:
        return "RIGHT";
    default:
        return "INVALID DIRECTION";
    }
}
int main() {
    int error = prepare_console(ENABLE_VIRTUAL_TERMINAL_PROCESSING, ENABLE_WINDOW_INPUT);
    if(error != 0) {
        printf("Unable to prepare console");
    }
    ConsoleColour col = {.fg = Red};
    STYLE s = Bold;
    write_styled_text(L"Hello World\n",s);
    //fflush(stdout);
    while(1) {
        EventList ev = get_event_list();
        //printf("Reading event list...");
        for(int i = 0; i < ev.length; i++) {
            INPUT_RECORD event = ev.inputRecord[i];
            switch (event.EventType)
            {
            case KEY_EVENT:
                if(event.Event.KeyEvent.bKeyDown) {
                    move_cursor(
                        char_to_direction(event.Event.KeyEvent.wVirtualKeyCode),
                        1
                    );
                }
                break;
            
            default:
                break;
            }
        }
    }
}

