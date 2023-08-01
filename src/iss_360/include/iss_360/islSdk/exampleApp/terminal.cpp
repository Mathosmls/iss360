//------------------------------------------ Includes ----------------------------------------------

#include "terminal.h"
#include "platformSetup.h"

#ifdef OS_WINDOWS
	#include "windows.h"
	#include <conio.h>
#else
	#include <stdlib.h>
	#include <string.h>
	#include <unistd.h>
	#include <sys/select.h>
	#include <termios.h>
#endif

#ifdef OS_WINDOWS
//--------------------------------------------------------------------------------------------------
void getExePath(char* argv0, char* path, int pathSize)
{
	_splitpath_s(argv0, 0, 0, path, pathSize, 0, 0, 0, 0);
}
//--------------------------------------------------------------------------------------------------
void resetTerminalMode()
{
}
//--------------------------------------------------------------------------------------------------
void setConioTerminalMode()
{
}
//--------------------------------------------------------------------------------------------------
int keyboardPressed()
{
	return _kbhit();
}
//--------------------------------------------------------------------------------------------------
int getKey()
{
	return _getch();
}
//--------------------------------------------------------------------------------------------------
#else	
struct termios orig_termios;
//--------------------------------------------------------------------------------------------------
void getExePath(char* argv0, char* path, int pathSize)
{
	realpath(argv0, path);
}
//--------------------------------------------------------------------------------------------------
void resetTerminalMode()
{
	tcsetattr(0, TCSANOW, &orig_termios);
}
//--------------------------------------------------------------------------------------------------
void setConioTerminalMode()
{
	struct termios new_termios;

	/* take two copies - one for now, one for later */
	tcgetattr(0, &orig_termios);
	memcpy(&new_termios, &orig_termios, sizeof(new_termios));

	/* register cleanup handler, and set the new terminal mode */
	atexit(resetTerminalMode);
	cfmakeraw(&new_termios);
	tcsetattr(0, TCSANOW, &new_termios);
}
//--------------------------------------------------------------------------------------------------
int keyboardPressed()
{
	struct timeval tv = { 0L, 0L };
	fd_set fds;
	FD_ZERO(&fds);
	FD_SET(0, &fds);
	return select(1, &fds, NULL, NULL, &tv) > 0;
}
//--------------------------------------------------------------------------------------------------
int getKey()
{
	int r;
	unsigned char c;
	if ((r = read(0, &c, sizeof(c))) < 0) {
		return r;
	} else {
		return c;
	}
}
//--------------------------------------------------------------------------------------------------
#endif