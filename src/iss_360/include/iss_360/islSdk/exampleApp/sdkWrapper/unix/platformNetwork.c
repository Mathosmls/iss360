//------------------------------------------ Includes ----------------------------------------------

#include "platformNetwork.h"
#include "platform/mem.h"
#include "platform/debug.h"
#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <errno.h>

//--------------------------------------- Public Constants -----------------------------------------

#define INVALID_SOCKET -1
#define SOCKET_ERROR -1

//----------------------------------------- Local Types --------------------------------------------

typedef int32_t SOCKET;

typedef struct
{
	SOCKET sock;
	bool_t isTcp;
	bool_t isServer;
	bool_t connected;
	uint32_t ipAddress;
	uint16_t port;
} lanConn_t;

//------------------------------------------- Globals ----------------------------------------------

//---------------------------------- Private Function Prototypes -----------------------------------

static SOCKET createTcpSocket(bool_t isServer, uint32_t ipAddress, uint16_t port);
static SOCKET createUdpSocket(bool_t isServer, uint32_t ipAddress, uint16_t port);
static SOCKET tcpAcceptConnection(SOCKET sock);

//--------------------------------------------------------------------------------------------------
bool_t platformNetworkInitialise(void)
{
	return TRUE;
}
//--------------------------------------------------------------------------------------------------
void platformNetworkDeinitialise(void)
{
}
//--------------------------------------------------------------------------------------------------
void* platformNetworkOpenSocket(bool_t isTcp, bool_t isServer, uint32_t ipAddress, uint16_t port)
{
	lanConn_t* lanConn;
	SOCKET sock;

	if (isTcp)
	{
		sock = createTcpSocket(isServer, ipAddress, port);
	}
	else
	{
		sock = createUdpSocket(isServer, ipAddress, port);
	}

	lanConn = NULLPTR;

	if (sock != INVALID_SOCKET)
	{
		lanConn = memAlloc(sizeof(lanConn_t));

		if (lanConn != NULLPTR)
		{
			lanConn->sock = sock;
			lanConn->isTcp = isTcp;
			lanConn->isServer = isServer;
			lanConn->connected = FALSE;
			lanConn->ipAddress = ipAddress;
			lanConn->port = port;
		}
		else
		{
			close(sock);
		}
	}

	return lanConn;
}
//--------------------------------------------------------------------------------------------------
void platformNetworkCloseSocket(void* socket)
{
	lanConn_t* lanConn;

	lanConn = (lanConn_t*)socket;

	shutdown(lanConn->sock, SHUT_WR);
	close(lanConn->sock);
	memFree(lanConn);
}
//--------------------------------------------------------------------------------------------------
socketStatus_t platformNetworkWrite(void* socket, const uint8_t* data, uint32_t* size, uint32_t ipAddress, uint16_t port)
{
	socketStatus_t socketStatus;
	struct sockaddr_in addr;
	lanConn_t* lanConn;
	int32_t result;
	fd_set waitSend;
	struct timeval timeout;

	lanConn = (lanConn_t*)socket;

	FD_ZERO(&waitSend);
	FD_SET(lanConn->sock, &waitSend);
	timeout.tv_sec = 0;
	timeout.tv_usec = 0;
	socketStatus = SOCKET_STATUS_OK;


	result = select(lanConn->sock + 1, NULL, &waitSend, NULL, &timeout);
	if (result != SOCKET_ERROR)
	{
		if (FD_ISSET(lanConn->sock, &waitSend))
		{
			addr.sin_family = AF_INET;
			addr.sin_port = htons(port);
			addr.sin_addr.s_addr = ipAddress;

			result = sendto(lanConn->sock, data, *size, 0, (struct sockaddr*) & addr, sizeof(addr));
			*size = result;
		}
		else
		{
			if (lanConn->isTcp)
			{
				socketStatus = SOCKET_STATUS_TCP_WAITING;
			}
			*size = 0;
		}
	}

	if (result == SOCKET_ERROR)
	{
		socketStatus = SOCKET_STATUS_ERROR;
		*size = 0;
		debugLog("network write falied with error %d\n", errno);
	}

	return socketStatus;
}
//--------------------------------------------------------------------------------------------------
socketStatus_t platformNetworkRead(void* socket, uint8_t* buf, uint32_t* size, uint32_t* ipAddress, uint16_t* port)
{
	socketStatus_t socketStatus;
	struct sockaddr_in fromAddress;
	uint32_t fromAddressSize;
	int32_t result;
	int32_t bytesRead;
	lanConn_t* lanConn;
	fd_set waitRecv;
	struct timeval timeout;

	lanConn = (lanConn_t*)socket;

	FD_ZERO(&waitRecv);
	FD_SET(lanConn->sock, &waitRecv);
	FD_SET(STDIN_FILENO, &waitRecv);
	timeout.tv_sec = 0;
	timeout.tv_usec = 0;
	*ipAddress = 0;
	*port = 0;
	bytesRead = 0;

	result = select(lanConn->sock + 1, &waitRecv, NULL, NULL, &timeout);
	if (result != SOCKET_ERROR)
	{
		socketStatus = SOCKET_STATUS_OK;
		if (FD_ISSET(lanConn->sock, &waitRecv))
		{
			if (lanConn->isTcp && lanConn->isServer && !lanConn->connected)
			{
				lanConn->sock = tcpAcceptConnection(lanConn->sock);
				lanConn->connected = lanConn->sock != INVALID_SOCKET;
				if (lanConn->connected)
				{
					socketStatus = SOCKET_STATUS_TCP_CONNECTED;
				}
			}
			else
			{
				fromAddressSize = sizeof(fromAddress);
				result = recvfrom(lanConn->sock, buf, *size, 0, (struct sockaddr*) & fromAddress, &fromAddressSize);
				if (result != SOCKET_ERROR)
				{
					*ipAddress = fromAddress.sin_addr.s_addr;
					*port = htons(fromAddress.sin_port);
					bytesRead = result;
					lanConn->connected = result != 0;

					if (lanConn->isTcp && !lanConn->connected)
					{
						socketStatus = SOCKET_STATUS_TCP_DISCONNECTED;
						shutdown(lanConn->sock, SHUT_WR);
						close(lanConn->sock);
						if (lanConn->isServer)
						{
							result = createTcpSocket(lanConn->isServer, lanConn->ipAddress, lanConn->port);
							lanConn->sock = result;
							socketStatus = SOCKET_STATUS_TCP_WAITING;
						}
					}
				}
			}
		}
	}

	*size = bytesRead;

	if (result == SOCKET_ERROR)
	{
		socketStatus = SOCKET_STATUS_ERROR;
		debugLog("network read falied with error %d\n", errno);
	}

	return socketStatus;
}
//--------------------------------------------------------------------------------------------------
bool_t platformNetworkConnected(void)
{
	return TRUE;
}
//--------------------------------------------------------------------------------------------------
static SOCKET createTcpSocket(bool_t isServer, uint32_t ipAddress, uint16_t port)
{
	struct sockaddr_in addr;
	SOCKET sock;
	int32_t flags;

	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = ipAddress;
	addr.sin_port = htons(port);

	sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (sock == INVALID_SOCKET)
	{
		debugLog("socket falied to create %d\n", errno);
		return INVALID_SOCKET;
	}

	flags = fcntl(sock, F_GETFL);
    if (flags == -1)
    {
        debugLog("Could not get socket flags\n");
        close(sock);
        return INVALID_SOCKET;
    }

    if (fcntl(sock, F_SETFL, flags | O_NONBLOCK) == -1)
    {
		debugLog("Cund not set socket to non-blocking mode\n");
		close(sock);
		return INVALID_SOCKET;
	}

	if (isServer)
	{
		if (bind(sock, (struct sockaddr*) & addr, sizeof(addr)) == SOCKET_ERROR)
		{
			debugLog("bind falied with error %d\n", errno);
			close(sock);
			return INVALID_SOCKET;
		}

		if (listen(sock, 1) == SOCKET_ERROR)
		{
			debugLog("listen falied with error %d\n", errno);
			close(sock);
			return INVALID_SOCKET;
		}
	}
	else
	{
		if (connect(sock, (struct sockaddr*) & addr, sizeof(addr)) == SOCKET_ERROR)
		{
			if (errno != EWOULDBLOCK)
			{
				debugLog("connect falied with error %d\n", errno);
				close(sock);
				return INVALID_SOCKET;
			}
		}
	}

	return sock;
}
//--------------------------------------------------------------------------------------------------
static SOCKET createUdpSocket(bool_t isServer, uint32_t ipAddress, uint16_t port)
{
	struct sockaddr_in addr;
	SOCKET sock;
	int32_t flags;
	int32_t iOptVal;
	int32_t rbufsize;

	sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (sock == INVALID_SOCKET)
	{
		debugLog("socket falied to create %d\n", errno);
		return INVALID_SOCKET;
	}

	flags = fcntl(sock, F_GETFL);
    if (flags == -1)
    {
        debugLog("Could not get socket flags\n");
        close(sock);
        return INVALID_SOCKET;
    }

    if (fcntl(sock, F_SETFL, flags | O_NONBLOCK) == -1)
    {
		debugLog("Cound not set socket to non-blocking mode\n");
		close(sock);
		return INVALID_SOCKET;
	}

	rbufsize = 1024 * 512;
	if (setsockopt(sock, SOL_SOCKET, SO_RCVBUF, (char*)&rbufsize, sizeof(rbufsize)) == SOCKET_ERROR)
	{
		debugLog("Cound not set socket rx buffer size\n");
	}

	if (isServer)
	{
		addr.sin_family = AF_INET;
		addr.sin_addr.s_addr = ipAddress;
		addr.sin_port = htons(port);

		if (bind(sock, (struct sockaddr*) & addr, sizeof(addr)) == SOCKET_ERROR)
		{
			debugLog("bind falied with error %d\n", errno);
			close(sock);
			return INVALID_SOCKET;
		}
	}
	else
	{
		iOptVal = 1;
		if (setsockopt(sock, SOL_SOCKET, SO_BROADCAST, (char*)&iOptVal, sizeof(iOptVal)) == SOCKET_ERROR)
		{
			debugLog("setsockopt falied with error %d\n", errno);
			close(sock);
			return INVALID_SOCKET;
		}
	}

	return sock;
}
//--------------------------------------------------------------------------------------------------
static SOCKET tcpAcceptConnection(SOCKET sock)
{
	SOCKET newSock;
	int32_t flags;

	newSock = accept(sock, NULL, NULL);

	if (newSock != INVALID_SOCKET)
	{
		shutdown(sock, SHUT_RDWR);
		close(sock);

		flags = fcntl(newSock, F_GETFL);
        if (flags == -1)
        {
            debugLog("Could not get socket flags\n");
            close(newSock);
            newSock = INVALID_SOCKET;
        }

        if (fcntl(sock, F_SETFL, flags | O_NONBLOCK) == -1)
        {
            debugLog("Cund not set socket to non-blocking mode\n");
            close(newSock);
            newSock = INVALID_SOCKET;
        }
	}

	return newSock;
}
//--------------------------------------------------------------------------------------------------
