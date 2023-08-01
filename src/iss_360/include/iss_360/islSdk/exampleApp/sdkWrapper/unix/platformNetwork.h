#ifndef PLATFORMNETWORK_H_
#define PLATFORMNETWORK_H_
#ifdef __cplusplus
extern "C" {
#endif

//------------------------------------------ Includes ----------------------------------------------

#include "platform/sdkTypes.h"
#include "platform/network.h"

//--------------------------------------- Public Constants -----------------------------------------

//----------------------------------------- Public Types -------------------------------------------

//---------------------------------- Public Function Prototypes ------------------------------------

bool_t platformNetworkInitialise(void);
void platformNetworkDeinitialise(void);
void* platformNetworkOpenSocket(bool_t isTcp, bool_t isServer, uint32_t ipAddress, uint16_t port);
void platformNetworkCloseSocket(void* socket);
socketStatus_t platformNetworkWrite(void* socket, const uint8_t* data, uint32_t* size, uint32_t ipAddress, uint16_t port);
socketStatus_t platformNetworkRead(void* socket, uint8_t* buf, uint32_t* size, uint32_t* ipAddress, uint16_t* port);
bool_t platformNetworkConnected(void);

//--------------------------------------------------------------------------------------------------
#ifdef __cplusplus
}
#endif
#endif
