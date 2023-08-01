//------------------------------------------ Includes ----------------------------------------------

#include "platformSetup.h"
#include "platform/sdkTypes.h"
#include "platform/file.h"
#include "platform/uart.h"
#include "platform/network.h"
#include "platform/timeUtils.h"

#ifdef OS_UNIX
#include "unix/platformFile.h"
#include "unix/platformUart.h"
#include "unix/platformNetwork.h"
#include "unix/platformTime.h"
#else
#ifdef OS_WINDOWS
#include "windows/platformFile.h"
#include "windows/platformUart.h"
#include "windows/platformNetwork.h"
#include "windows/platformTime.h"
#endif
#endif

//--------------------------------------- Public Constants -----------------------------------------

//----------------------------------------- Local Types --------------------------------------------

//------------------------------------------- Globals ----------------------------------------------

//---------------------------------- Private Function Prototypes -----------------------------------

//--------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
void platformSetup(void)
{
	fileFunctions.create = &platformFileCreate;
	fileFunctions.open = &platformFileOpen;
	fileFunctions.close = &platformFileClose;
	fileFunctions.seek = &platformFileSeek;
	fileFunctions.read = &platformFileRead;
	fileFunctions.readLine = &platformFileReadLineUtf8;
	fileFunctions.write = &platformFileWrite;

	timeFunctions.getTime = &platformTimeGet;
	timeFunctions.convert = &platformTimeConvert;
	timeFunctions.getTicks = &platformTimeGetTicks;
	timeFunctions.getTickFrequency = &platformTimeGetTickFrequency;
	
    uartFunctions.getPortNames = &platformUartGetPorts;
    uartFunctions.open = &platformUartOpen;
    uartFunctions.close = &platformUartClose;
    uartFunctions.config = &platformUartConfig;
    uartFunctions.write = &platformUartWrite;
    uartFunctions.read = &platformUartRead;

	networkFunctions.initialise = &platformNetworkInitialise;
	networkFunctions.deinitialise = &platformNetworkDeinitialise;
	networkFunctions.openSocket = &platformNetworkOpenSocket;
	networkFunctions.closeSocket = &platformNetworkCloseSocket;
	networkFunctions.write = &platformNetworkWrite;
	networkFunctions.read = &platformNetworkRead;
	networkFunctions.connected = &platformNetworkConnected;
}
//--------------------------------------------------------------------------------------------------
