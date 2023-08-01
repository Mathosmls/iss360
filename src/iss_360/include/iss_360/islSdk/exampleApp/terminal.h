#ifndef TERMINAL_H_
#define TERMINAL_H_

//------------------------------------------ Includes ----------------------------------------------

//--------------------------------------- Class Definition -----------------------------------------

void getExePath(char* argv0, char* path, int pathSize);
void resetTerminalMode();
void setConioTerminalMode();
int keyboardPressed();
int getKey();

//--------------------------------------------------------------------------------------------------
#endif