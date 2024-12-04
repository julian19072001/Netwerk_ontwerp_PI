// ncurses_helloworld.cpp
//
// g++ -Wall -o ncurses_helloworldcpp ncurses_helloworld.cpp -lncurses
// ./ncurses_helloworldcpp
//
// Author : Edwin Boer
// Version: 20200330

#include <ncurses.h>

int main(int nArgc, char* aArgv[]) {

  // Start curses mode
  initscr();

  // Print Hello World
  printw("Hello World with ncurses C++ :-)");
  // Print it on to the real screen
  refresh();

  // Read: wait for user input
  getch();
  // End curses mode
  endwin();

  return 0;
};
