// ncurses_helloworld.c
//
// gcc -Wall -o ncurses_helloworldc ncurses_helloworld.c -lncurses
// ./ncurses_helloworldc
//
// Author : Edwin Boer
// Version: 20200330

#include <ncurses.h>

int main(int nArgc, char* aArgv[]) {

  // Start curses mode
  initscr();

  // Print Hello World
  printw("Hello World with ncurses C :-)");
  // Print it on to the real screen
  refresh();

  // Read: wait for user input
  getch();
  // End curses mode
  endwin();

  return 0;
};
