#ifndef NETWORK_H_
#define NETWORK_H_
#ifdef __cplusplus
extern "C" {
#endif
//------------------------------------------ Includes ----------------------------------------------

#include "sdkTypes.h"

//--------------------------------------- Public Constants -----------------------------------------

//----------------------------------------- Public Types ------------------------------------------

typedef enum
{
    SOCKET_STATUS_OK,
    SOCKET_STATUS_TCP_WAITING,
    SOCKET_STATUS_TCP_CONNECTED,
    SOCKET_STATUS_TCP_DISCONNECTED,
    SOCKET_STATUS_ERROR,
} socketStatus_t;

typedef void socket_t;

/// Platform network function pointers
typedef struct
{
    /***************************************************************************************************
    * Function pointer to Initialise the network
    * @return True if succeeded
    ***************************************************************************************************/
    bool_t (*initialise)(void);

    /***************************************************************************************************
    * Function pointer to De-initialise the network
    ***************************************************************************************************/
    void (*deinitialise)(void);

    /***************************************************************************************************
    * Function pointer to open a socket
    * @Param tcp - TRUE open a TCP socket
    *            - FALSE open a UDP socket
    * @Param isServer - TRUE if the socket is a server, (listen for data before sending)
    *                 - FALSE if the socket is a client, (send data first)
    * @Param ipAddress - In server TCP mode: the IP Address to listen for a connection from.
    *                  - In client TCP mode: the IP Address to connect and send data to.
    *                  - In server UDP mode: the IP Address to allow data from. 0 = any IP
    *                  - In client UDP mode: not used. Specify the IP address when sending data
    * @Param port - In server TCP mode: the port to listen for a connection from.
    *             - In client TCP mode: the port to connect and send data to.
    *             - In server UDP mode: the port to allow data from. 0 = any port
    *             - In client UDP mode: not used. Specify the port when sending data
    * @return Pointer to an object
    ***************************************************************************************************/
    socket_t* (*openSocket)(bool_t isTcp, bool_t isServer, uint32_t ipAddress, uint16_t port);

    /***************************************************************************************************
    * Function pointer to close a UDP port
    * @param inst Pointer the object return by openUdp()
    ***************************************************************************************************/
    void (*closeSocket)(socket_t* inst);

    /***************************************************************************************************
    * Function pointer to write to a UDP port
    * @param inst Pointer the object return by openUdp()
    * @param data Pointer of data to write
    * @param size Pointer to the number of bytes to write. This is updated with the number of bytes written
    * @param ipAddress IPv4 address to send the data to
    * @param dstPort Remote end destination port
    ***************************************************************************************************/
    socketStatus_t(*write)(socket_t* inst, const uint8_t* data, uint32_t* size, uint32_t ipAddress, uint16_t dstPort);

    /***************************************************************************************************
    * Function pointer to read to a UDP port
    * @param inst Pointer the object return by openUdp()
    * @param buf Pointer to buf in which to read into
    * @param size Pointer to the size of \p buf in bytes. This is updated with the number of bytes read
    * @param ipAddress Pointer to receive the senders IPv4 address
    * @param port Pointer to receive the senders source port number
    ***************************************************************************************************/
    socketStatus_t(*read)(socket_t* inst, uint8_t* buf, uint32_t* size, uint32_t* ipAddress, uint16_t* port);

    /***************************************************************************************************
    * Function pointer to check if a system has network capability
    * @return True if the system has network capability
    ***************************************************************************************************/
    bool_t (*connected)(void);
} networkFunctions_t;

extern networkFunctions_t networkFunctions;

//---------------------------------- Public Function Prototypes -----------------------------------

//--------------------------------------------------------------------------------------------------
#ifdef __cplusplus
}
#endif
#endif
