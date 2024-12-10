/*
* raw(?) I/O
*/
#include "io.h"
#include "common.h"

void gotoxy(POSITION pos) {
    if (pos.row < 0 || pos.column < 0) return;
    COORD coord = { pos.column, pos.row };
    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), coord);
}

void set_color(int color) {
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), color);
}

void printc(POSITION pos, char ch, int color) {
    if (color >= 0) {
        set_color(color);
    }
    gotoxy(pos);
    printf("%c", ch);
}

KEY get_key(void) {
    if (!_kbhit()) {
        return k_none;
    }

    int byte = _getch();
    switch (byte) {
    case 'q': return k_quit;
    case ' ': return k_space;
    case 27: return k_escape;
    case 'B': case 'b': return 'B';
    case 'P': case 'p': return 'P';
    case 'H': case 'h': return 'H';
    case 224: {
        byte = _getch();
        switch (byte) {
        case 72: return k_up;
        case 75: return k_left;
        case 77: return k_right;
        case 80: return k_down;
        default:
            printf("Undefined key pressed: %d\n", byte);
            return k_undef;
        }
    }
    default:
        printf("Undefined key pressed: %d\n", byte);
        return k_undef;
    }
}