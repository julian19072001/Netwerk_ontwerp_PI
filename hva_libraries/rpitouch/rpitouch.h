// rpitouch.h
//
// Use the RPi 7" Touchscreen in combination with ncurses.
//
// Used docu:
//  https://www.linuxjournal.com/article/6429
//  https://www.kernel.org/doc/html/v4.17/input/input_uapi.html
//  https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/tree/include/uapi/linux/input-event-codes.h
//
// Example: 
//  $ gcc -Wall -o ncurses_rpitouch ncurses_rpitouch.c ~/hva_libraries/rpitouch/*.c -I/home/piuser/hva_libraries/rpitouch -lncurses
//  $ ./ncurses_rpitouch    
//
// Author : Edwin Boer
// Version: 20200330

#ifndef _RPITOUCH_H_
#define _RPITOUCH_H_

  // System include files
  #include <stdio.h>
  #include <sys/types.h>
  #include <sys/ioctl.h>
  #include <dirent.h>
  #include <linux/input.h>
  #include <fcntl.h>
  #include <unistd.h>
  #include <errno.h>
  #include <string.h>
  #include <stdbool.h>
  #include <stdint.h>
  #include <stdlib.h>

  // Library defines and previous device names: 
  //      "FT5406 memory based driver"
  //      "raspberrypi-ts"
  // now: "generic ft5x06 (79)"
  #define RPITOUCH_DEVICE_SLOT_COUNT 10
  #define RPITOUCH_DEVICE_NAME "generic ft"
  #define RPITOUCH_DEVICE_WIDTH_PIX 800
  #define RPITOUCH_DEVICE_HEIGHT_PIX 480
  #define RPITOUCH_SWIPE_WIDTH_COUNT 3
  #define RPITOUCH_SWIPE_HEIGHT_COUNT 3
  #define RPITOUCH_SCRIPT_RESTART  "~/hva_libraries/rpitouch/shellscripts/rpitouch_restart.sh"
  #define RPITOUCH_SCRIPT_SHUTDOWN "~/hva_libraries/rpitouch/shellscripts/rpitouch_shutdown.sh"

  // Helper structs
  struct RPiTouch_MultiTouchSlot {
    bool bUsed;       // false: not active used, true: active
    uint32_t nId;     // EV_ABS ABS_MT_TRACKING_ID
    uint32_t nX;      // EV_ABS ABS_MT_POSITION_X
    uint32_t nY;      // EV_ABS ABS_MT_POSITION_Y
    uint32_t nCol;    // column 
    uint32_t nRow;    // row
  };
  typedef struct RPiTouch_MultiTouchSlot RPiTouch_MultiTouchSlot_t;

  struct RPiTouch_Touch {
    bool bButton;     // false: no touch, true: touched
    uint32_t nX;      // EV_ABS ABS_X
    uint32_t nY;      // EV_ABS ABS_Y
    uint32_t nCol;    // column
    uint32_t nRow;    // row
  };
  typedef struct RPiTouch_Touch RPiTouch_Touch_t;

  struct RPiTouch_Settings {
    // Restart settings and status
    bool bRestartApply;           // Apply restart if detected, otherwise marking only
    uint8_t nRestartWait;         // Waiting time in seconds
    bool bRestartDetected;        // Restart detected status
    uint8_t aRestartSwipe[9];     // Swipe route
    uint8_t nRestartSwipeSize;    // Swipe route length
    uint8_t nRestartSwipeState;   // Swipe route state

    // Shutdown settings and status
    bool bShutdownApply;          // Apply shutdown if detected, otherwise marking only
    uint8_t nShutdownWait;        // Waiting time in seconds
    bool bShutdownDetected;       // Shutdown detected status
    uint8_t aShutdownSwipe[9];    // Swipe route
    uint8_t nShutdownSwipeSize;   // Swipe route length
    uint8_t nShutdownSwipeState;  // Swipe route state
  };
  typedef struct RPiTouch_Settings RPiTouch_Settings_t;

  // Global status variables
  extern RPiTouch_Settings_t _oRPiTouch_Settings;
  extern RPiTouch_MultiTouchSlot_t _aRPiTouch_Slot[RPITOUCH_DEVICE_SLOT_COUNT];
  extern RPiTouch_Touch_t _oRPiTouch_Touched;
  extern struct winsize _oRPiTouch_TerminalSize;
  extern bool _oRPiTouch_TerminalUsed;
  extern int _nRPiTouch_EventId;
  extern int _nRPiTouch_EventStream;
  extern int _nRPiTouch_EventStream_CurSlot;

  // Library main access
  int RPiTouch_InitTouch();
  bool RPiTouch_UpdateTouch();
  int RPiTouch_CloseTouch();

  // Library swipe controll
  void RPiTouch_ApplyRestart();
  void RPiTouch_ApplyShutdown();
  void RPiTouch_UpdateSwipe();

#endif // _RPITOUCH_H_