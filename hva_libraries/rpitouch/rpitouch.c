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

#include <rpitouch.h>

// Global status variables
RPiTouch_Settings_t _oRPiTouch_Settings;
RPiTouch_MultiTouchSlot_t _aRPiTouch_Slot[RPITOUCH_DEVICE_SLOT_COUNT];
RPiTouch_Touch_t _oRPiTouch_Touched;
struct winsize _oRPiTouch_TerminalSize;
bool _oRPiTouch_TerminalUsed;
int _nRPiTouch_EventId;
int _nRPiTouch_EventStream;
int _nRPiTouch_EventStream_CurSlot;

/** -----------------------------------
 * 
 */
int RPiTouch_InitTouch() {

  DIR *pDir;
  struct dirent *pDirItem;
  int nDevice;
  char sFilename[1025];
  char sDeviceName[256];
  uint8_t aRestartSwipe[] = {0, 1, 2, 5, 8};
  uint8_t aShutdownSwipe[] = {0, 3, 6, 7, 8};

  // Init lib defaults for restart
  _oRPiTouch_Settings.bRestartApply = false;
  _oRPiTouch_Settings.nRestartWait = 5;
  _oRPiTouch_Settings.bRestartDetected = false;
  _oRPiTouch_Settings.nRestartSwipeSize = 5;
  memcpy(_oRPiTouch_Settings.aRestartSwipe, aRestartSwipe, _oRPiTouch_Settings.nRestartSwipeSize);
  _oRPiTouch_Settings.nRestartSwipeState = 0;

  // Init lib defaults for shutdown
  _oRPiTouch_Settings.bShutdownApply = true;
  _oRPiTouch_Settings.nShutdownWait = 5;
  _oRPiTouch_Settings.bShutdownDetected = false;
  _oRPiTouch_Settings.nShutdownSwipeSize = 5;
  memcpy(_oRPiTouch_Settings.aShutdownSwipe, aShutdownSwipe, _oRPiTouch_Settings.nShutdownSwipeSize);
  _oRPiTouch_Settings.nShutdownSwipeState = 0;

  // Init global vars
  memset(_aRPiTouch_Slot, 0, sizeof(RPiTouch_MultiTouchSlot_t) * RPITOUCH_DEVICE_SLOT_COUNT);
  memset(&_oRPiTouch_Touched, 0, sizeof(RPiTouch_Touch_t));
  _nRPiTouch_EventId = -1;
  _nRPiTouch_EventStream = -1;
  _nRPiTouch_EventStream_CurSlot = 0;

  // Try to open the Linux Input directory
  pDir = opendir("/dev/input");
  if (pDir == NULL) {
    //printf("Error: could not open /dev/input\n\n");
    return -2;
  }

  // Check all "event#"
  _nRPiTouch_EventId = 0;
  while ((pDirItem = readdir(pDir)) != NULL) {

    // Parse directory item for: event#
    if (sscanf(pDirItem->d_name, "event%u", &_nRPiTouch_EventId) == 1) {
      printf("%d %s\n", _nRPiTouch_EventId, pDirItem->d_name);

      // Get the device information
      sprintf(sFilename, "/dev/input/%s", pDirItem->d_name);
      nDevice = open(sFilename, O_RDONLY | O_NONBLOCK);
      if (nDevice <= 0) {
        //printf("Error: unable to open event-stream\n\n");
        return -3;
      }
      else {
        // Event-stream is open
        if (ioctl(nDevice, EVIOCGNAME(256), sDeviceName) >= 0) {
          // Tip: use next debug line to determine new touchscreen name
          //printf("Device [%s] with name [%s]\n", pDirItem->d_name, sDeviceName);

          if (strncmp(sDeviceName, RPITOUCH_DEVICE_NAME, strlen(RPITOUCH_DEVICE_NAME)) == 0) {
            // Found :-) so no closing needed
            _nRPiTouch_EventStream = nDevice;

            // Return the event number
            return _nRPiTouch_EventId;
          }
        }

        // Close event-stream
        close(nDevice);
      }
    }   
  }

  // Close directory read
  closedir(pDir);

  // None found
  //printf("Error: could not find the touchscreen\n\n");
  return -1;
}

/** -----------------------------------
 * 
 */
bool RPiTouch_UpdateTouch() {

  bool bUpdate = false;
  struct input_event oEvent;
  ssize_t nReadCount;

  // A valid event-stream?
  if (_nRPiTouch_EventStream < 0) {
    return false;
  }

  // Determine terminal size in chr and pix
  ioctl(0, TIOCGWINSZ, &_oRPiTouch_TerminalSize);
  if (_oRPiTouch_TerminalSize.ws_xpixel == 0 && _oRPiTouch_TerminalSize.ws_ypixel == 0) {
    _oRPiTouch_TerminalUsed = false;
    _oRPiTouch_TerminalSize.ws_xpixel = RPITOUCH_DEVICE_WIDTH_PIX;
    _oRPiTouch_TerminalSize.ws_ypixel = RPITOUCH_DEVICE_HEIGHT_PIX;
  }
  else {
    _oRPiTouch_TerminalUsed = true;
  }

  // Any messages to read? error 11 EAGAIN = Try again (= no data yet because of no-blocking)
  while (!(nReadCount = read(_nRPiTouch_EventStream, &oEvent, sizeof(struct input_event)) == -1 && errno == 11)) {
    switch (oEvent.type) {
        case EV_SYN: // SYN_REPORT
          bUpdate = true;
          break;

        case EV_KEY: 
          switch (oEvent.code) {
            case BTN_TOUCH:
              if (oEvent.value > 0) {
                // Start touching
                _oRPiTouch_Touched.bButton = true;
              }
              else {
                // No toyching
                _oRPiTouch_Touched.bButton = false;

                // Clear all touch data
                for (int i = 0; i < RPITOUCH_DEVICE_SLOT_COUNT; i++) {
                  _aRPiTouch_Slot[_nRPiTouch_EventStream_CurSlot].bUsed = false;
                  _aRPiTouch_Slot[_nRPiTouch_EventStream_CurSlot].nId = 0;
                }
              }
              bUpdate = true;
              break;
          }
          break;

        case EV_REL: 
          break;

        case EV_ABS: 
          switch (oEvent.code) {
            case ABS_X:
              _oRPiTouch_Touched.nX = oEvent.value;
              _oRPiTouch_Touched.nCol = (_oRPiTouch_Touched.nX * _oRPiTouch_TerminalSize.ws_col) / _oRPiTouch_TerminalSize.ws_xpixel;
              bUpdate = true;
              break;

            case ABS_Y:
              _oRPiTouch_Touched.nY = oEvent.value;
              _oRPiTouch_Touched.nRow = (_oRPiTouch_Touched.nY * _oRPiTouch_TerminalSize.ws_row) / _oRPiTouch_TerminalSize.ws_ypixel;
              bUpdate = true;
              break;

            case ABS_MT_SLOT:
              _nRPiTouch_EventStream_CurSlot = oEvent.value;
              bUpdate = true;
              break;

            case ABS_MT_TRACKING_ID:
              if (oEvent.value == 0xffffffff) {
                _aRPiTouch_Slot[_nRPiTouch_EventStream_CurSlot].bUsed = false;
                _aRPiTouch_Slot[_nRPiTouch_EventStream_CurSlot].nId = 0;
              }
              else {
                _aRPiTouch_Slot[_nRPiTouch_EventStream_CurSlot].bUsed = true;
                _aRPiTouch_Slot[_nRPiTouch_EventStream_CurSlot].nId = oEvent.value;
              }
              bUpdate = true;
              break;

            case ABS_MT_POSITION_X:
              _aRPiTouch_Slot[_nRPiTouch_EventStream_CurSlot].bUsed = true;
              _aRPiTouch_Slot[_nRPiTouch_EventStream_CurSlot].nX = oEvent.value;
              _aRPiTouch_Slot[_nRPiTouch_EventStream_CurSlot].nCol = (_aRPiTouch_Slot[_nRPiTouch_EventStream_CurSlot].nX * _oRPiTouch_TerminalSize.ws_col) / _oRPiTouch_TerminalSize.ws_xpixel;
              bUpdate = true;
              RPiTouch_UpdateSwipe();
              break;

            case ABS_MT_POSITION_Y:
              _aRPiTouch_Slot[_nRPiTouch_EventStream_CurSlot].bUsed = true;
              _aRPiTouch_Slot[_nRPiTouch_EventStream_CurSlot].nY = oEvent.value;
              _aRPiTouch_Slot[_nRPiTouch_EventStream_CurSlot].nRow = (_aRPiTouch_Slot[_nRPiTouch_EventStream_CurSlot].nY * _oRPiTouch_TerminalSize.ws_row) / _oRPiTouch_TerminalSize.ws_ypixel;
              bUpdate = true;
              RPiTouch_UpdateSwipe();
              break;
          }
          break;
    }
  }

  return bUpdate;
}

/** -----------------------------------
 * 
 */
int RPiTouch_CloseTouch() {

  return close(_nRPiTouch_EventStream);
}

/** -----------------------------------
 * 
 */
void RPiTouch_ApplyRestart() {

  // NO clear screen and start applying after countdown
  printf("\n\nApply restart ... "); fflush(stdout);
  for (uint8_t t = _oRPiTouch_Settings.nRestartWait; t > 0; t--) {
    printf("%d ", t); fflush(stdout);
    sleep(1);
  }
  printf("now!\n\n"); fflush(stdout);
  system(RPITOUCH_SCRIPT_RESTART);

  exit(1);
}

/** -----------------------------------
 * 
 */
void RPiTouch_ApplyShutdown() {

  // NO clear screen and start applying after countdown
  printf("\n\nApply shutdown ... "); fflush(stdout);
  for (uint8_t t = _oRPiTouch_Settings.nShutdownWait; t > 0; t--) {
    printf("%d ", t); fflush(stdout);
    sleep(1);
  }
  printf("now!\n\n"); fflush(stdout);
  system(RPITOUCH_SCRIPT_SHUTDOWN);

  exit(2);
}

/** -----------------------------------
 * 
 */
void RPiTouch_UpdateSwipe() {

  uint16_t nWidth, nZoneWidth;
  uint16_t nHeight, nZoneHeight;
  uint16_t nCurrentX, nCurrentY;
  uint16_t nZoneNr;

  // RPITOUCH_SWIPE_WIDTH_COUNT 3
  // RPITOUCH_SWIPE_HEIGHT_COUNT 3
  //
  // Swipezones:  0 1 2
  //              3 4 5
  //              6 7 8
  // A swipe:
  // - Start slot 0 holding in zone 0
  // - Move route in slot 1 over multiple zones
  
  nWidth       = RPITOUCH_DEVICE_WIDTH_PIX; // Not: _oRPiTouch_TerminalSize.ws_xpixel;
  nZoneWidth   = nWidth / RPITOUCH_SWIPE_WIDTH_COUNT;
  nHeight      = RPITOUCH_DEVICE_HEIGHT_PIX; // Not: _oRPiTouch_TerminalSize.ws_ypixel;
  nZoneHeight  = nHeight / RPITOUCH_SWIPE_HEIGHT_COUNT;

  // Two slots required
  if (RPITOUCH_DEVICE_SLOT_COUNT < 2) {
    return;
  }

  // Active slot 0 in zone 1?
  if (!_aRPiTouch_Slot[0].bUsed || _aRPiTouch_Slot[0].nX > nZoneWidth || _aRPiTouch_Slot[0].nY > nZoneHeight) {
    return;
  }

  // Active slot 1?
  if (!_aRPiTouch_Slot[1].bUsed) {

    // Reset the swipes
    _oRPiTouch_Settings.nRestartSwipeState = 0;
    _oRPiTouch_Settings.nShutdownSwipeState = 0;
    return;
  }

  // Use curremt x,y that are always from the touchscreen (not from the terminal) to calculate the zone
  nCurrentX = _aRPiTouch_Slot[1].nX;
  nCurrentY = _aRPiTouch_Slot[1].nY;
  nZoneNr = (nCurrentY / nZoneHeight) * RPITOUCH_SWIPE_WIDTH_COUNT + (nCurrentX / nZoneWidth); 

  //mvprintw(8,45,"wxh %d x %d  T(%d x %d)  zone wxh %d x %d        ", nWidth, nHeight, _oRPiTouch_TerminalSize.ws_xpixel, _oRPiTouch_TerminalSize.ws_ypixel, nZoneWidth, nZoneHeight);
  //mvprintw(10,45,"zone xy %d,%d   xy %d %d   = %3d ", nCurrentX, nCurrentY, (nCurrentX / nZoneWidth), (nCurrentY / nZoneHeight), nZoneNr); 
  //refresh();

  // Update the swipe?
  if (_oRPiTouch_Settings.aRestartSwipe[_oRPiTouch_Settings.nRestartSwipeState] == nZoneNr) {
    // Still in the current swipe zone; no action needed
  }
  else if (_oRPiTouch_Settings.aRestartSwipe[_oRPiTouch_Settings.nRestartSwipeState + 1] == nZoneNr) {
    // Go to next swipe state
    _oRPiTouch_Settings.nRestartSwipeState++;

    // It is the last swipe state?
    if (_oRPiTouch_Settings.nRestartSwipeState + 1 == _oRPiTouch_Settings.nRestartSwipeSize) {

      _oRPiTouch_Settings.nRestartSwipeState = 0;  
      _oRPiTouch_Settings.bRestartDetected = true;

      // Apply?
      if (_oRPiTouch_Settings.bRestartApply) {
        RPiTouch_ApplyRestart();
      }
    }
  }
  else {
    // Wrong swipe zones
    _oRPiTouch_Settings.nRestartSwipeState = 0;
  }
  //mvprintw(12,45,"restart state %d of %d ", _oRPiTouch_Settings.nRestartSwipeState, _oRPiTouch_Settings.nRestartSwipeSize); 
  //refresh();

  // Update the swipe?
  if (_oRPiTouch_Settings.aShutdownSwipe[_oRPiTouch_Settings.nShutdownSwipeState] == nZoneNr) {
    // Still in the current swipe zone; no action needed
  }
  else if (_oRPiTouch_Settings.aShutdownSwipe[_oRPiTouch_Settings.nShutdownSwipeState + 1] == nZoneNr) {
    // Go to next swipe state
    _oRPiTouch_Settings.nShutdownSwipeState++;

    // It is the last swipe state?
    if (_oRPiTouch_Settings.nShutdownSwipeState + 1 == _oRPiTouch_Settings.nShutdownSwipeSize) {

      _oRPiTouch_Settings.nShutdownSwipeState = 0;  
      _oRPiTouch_Settings.bShutdownDetected = true;

      // Apply?
      if (_oRPiTouch_Settings.bShutdownApply) {
        RPiTouch_ApplyShutdown();
      }
    }
  }
  else {
    // Wrong swipe zones
    _oRPiTouch_Settings.nShutdownSwipeState = 0;
  }
  //mvprintw(12,45,"Shutdown state %d of %d ", _oRPiTouch_Settings.nShutdownSwipeState, _oRPiTouch_Settings.nShutdownSwipeSize); 
  //refresh();
}
