#include <ncurses.h>
#include <stdio.h>

int main() {
    initscr();
    if (!has_colors()) {
        endwin();
        printf("This terminal does not support colors.\n");
        return 1;
    }

    start_color();
    if (!can_change_color()) {
        endwin();
        printf("This terminal does not support custom colors.\n");
        return 1;
    }

    // Define a custom color
    short custom_color = 8;
    init_color(custom_color, 500, 250, 750);  // RGB: 50%, 25%, 75%
    init_pair(1, custom_color, COLOR_BLACK); // Custom foreground on black

    // Display result
    attron(COLOR_PAIR(1));
    printw("Custom RGB color is supported!\n");
    attroff(COLOR_PAIR(1));
    refresh();

    getch();  // Wait for input
    endwin();
    return 0;
}
