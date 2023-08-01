//------------------------------------------ Includes ----------------------------------------------

#include "platformUart.h"
#include "platform/debug.h"
#include <string.h>
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <linux/serial.h>

// Ensure this app has permission to access serial ports. Add the user to the dialout group. "sudo usermod -a -G dialout MY_USER_NAME"

//--------------------------------------- Public Constants -----------------------------------------

//----------------------------------------- Local Types --------------------------------------------

//------------------------------------------- Globals ----------------------------------------------

//---------------------------------- Private Function Prototypes -----------------------------------

static int32_t strCopyMax(uint8_t* dst, const uint8_t* src, int32_t maxLength);
static speed_t convertToUnixBaudRate(uint32_t baudrate);

//--------------------------------------------------------------------------------------------------
uint32_t platformUartGetPorts(utf8_t* buf, uint32_t bufSize)
{
    DIR* dirh;
    struct serial_struct serinfo;
    struct dirent* fileInfo;
    struct stat st;
    int32_t fd;
    uint8_t path[1024] = "/sys/class/tty";
    uint8_t* ttyName;
    int32_t ttyNameSize;
    uint8_t* str;
    int32_t length;
    uint32_t count = 0;
    bool_t isHardwarePort;


    dirh = opendir((char*)&path[0]);
    if (dirh == NULLPTR || bufSize < 10)
    {
        *buf = 0;
        return 0;
    }

    bufSize -= 2;
    ttyName = &path[strlen((char*)&path[0])];
    strCopyMax(ttyName, (uint8_t*)"/", 2);
    ttyName++;

    while ((fileInfo = readdir(dirh)) != 0)
    {
        if (fileInfo->d_name[0] == 't' && fileInfo->d_name[1] == 't' && fileInfo->d_name[2] == 'y')
        {
            ttyNameSize = strCopyMax(ttyName, (uint8_t*)fileInfo->d_name, sizeof(path) - (uint32_t)(ttyName - &path[0]));
            str = ttyName + ttyNameSize;

            str += strCopyMax(str, (uint8_t*)"/device", sizeof(path) - (uint32_t)(str - &path[0]));

            if (lstat((char*)&path[0], &st) == 0 && S_ISLNK(st.st_mode))
            {
                str += strCopyMax(str, (uint8_t*)"/driver", sizeof(path) - (uint32_t)(str - &path[0]));
                length = readlink((char*)&path[0], (char*)str, sizeof(path) - (uint32_t)(str - &path[0]) - 1);

                if (length > 0)
                {
                    str[length] = 0;
                    str = (uint8_t*)strrchr((char*)str, '/');
                    if (str++ != NULLPTR)
                    {
                        isHardwarePort = !strcmp((char*)str, (char*)"serial8250");
                        str = buf;
                        length = strCopyMax(buf, (uint8_t*)"/dev/", bufSize);

                        if (ttyNameSize < (bufSize - length))
                        {
                            length += strCopyMax(buf + length, ttyName, ttyNameSize) + 1;
                            bufSize -= length;
                            buf += length;
                            count++;

                            if (isHardwarePort)
                            {
                                fd = open((char*)str, O_RDWR | O_NONBLOCK | O_NOCTTY);

                                if (fd >= 0)
                                {
                                    if (ioctl(fd, TIOCGSERIAL, &serinfo) == 0)
                                    {
                                        if (serinfo.type == PORT_UNKNOWN)
                                        {
                                            buf = str;
                                            bufSize += length;
                                            count--;
                                        }
                                    }
                                    close(fd);
                                }
                                else
                                {
                                    buf = str;
                                    bufSize += length;
                                    count--;
                                }
                            }
                        }
                    }
                }
            }
        }
        *ttyName = 0;
    }

    closedir(dirh);
    *buf = 0;

    return count;
}
//--------------------------------------------------------------------------------------------------
static int32_t strCopyMax(uint8_t* dst, const uint8_t* src, int32_t maxLength)
{
    int32_t count = 0;

    while (*src && (count < maxLength))
    {
        *dst++ = *src++;
        count++;
    }
    *dst = 0;

    return count;
}
//--------------------------------------------------------------------------------------------------
void* platformUartOpen(const utf8_t* portStr)
{
    int32_t port;

    port = open((char*)portStr, O_RDWR | O_NOCTTY | O_SYNC);

    if (port > 0)
    {
        if (!platformUartConfig(port, 9600, UART_NO_PARITY, 8, UART_1_STOPBIT))
        {
            port = 0;
        }
    }
    else
    {
        port = 0;
        debugLog("Error opening uart %s: %s", portStr, strerror(errno));
    }

    return (void*)port;
}
//--------------------------------------------------------------------------------------------------
void platformUartClose(void* handle)
{
    int32_t port;

    port = (int32_t)handle;

    close(port);
}
//--------------------------------------------------------------------------------------------------
bool_t platformUartConfig(void* handle, uint32_t baudrate, uint8_t dataBits, uartParity_t parity, uartStopBits_t stopBits)
{
    int32_t port;
    struct termios tty;
    uint32_t parityLut[] = { 0, PARENB | PARODD, PARENB, 0 , 0 };
    uint32_t dataBitsLut[] = { CS5, CS6, CS7, CS8 };
    uint32_t stopBitsLut[] = { 0, CSTOPB, CSTOPB };

    port = (int32_t)handle;

    if (dataBits < 5 || dataBits > 8)
    {
        dataBits = 8;
    }
    dataBits -= 5;

    if (tcgetattr(port, &tty) < 0)
    {
        debugLog("Error from uart tcgetattr: %s", strerror(errno));
        return FALSE;
    }

    cfsetospeed(&tty, convertToUnixBaudRate(baudrate));
    cfsetispeed(&tty, convertToUnixBaudRate(baudrate));

    tty.c_iflag = 0;
    tty.c_oflag = 0;
    tty.c_cflag = CLOCAL | CREAD | dataBitsLut[dataBits] | parityLut[parity] | stopBitsLut[stopBits];
    tty.c_lflag = 0;

    tty.c_cc[VMIN] = 0;
    tty.c_cc[VTIME] = 1;

    if (tcsetattr(port, TCSANOW, &tty))
    {
        debugLog("Error from uart tcsetattr: %s", strerror(errno));
        return FALSE;
    }

    return TRUE;
}
//--------------------------------------------------------------------------------------------------
static speed_t convertToUnixBaudRate(uint32_t baudrate)
{
    if (baudrate == 115200) return B115200;
    if (baudrate == 57600) return B57600;
    if (baudrate == 38400) return B38400;
    if (baudrate == 19200) return B19200;
    if (baudrate == 9600) return B9600;
    if (baudrate == 4800) return B4800;
    if (baudrate == 2400) return B2400;
    if (baudrate == 1800) return B1800;
    if (baudrate == 1200) return B1200;
    if (baudrate == 600) return B600;
    if (baudrate == 300) return B300;
    if (baudrate == 200) return B200;
    if (baudrate == 150) return B150;
    if (baudrate == 134) return B134;
    if (baudrate == 110) return B110;
    if (baudrate == 75) return B75;
    if (baudrate == 50) return B50;
    return B0;
}
//--------------------------------------------------------------------------------------------------
bool_t platformUartWrite(void* handle, const uint8_t* data, uint32_t* size)
{
    int32_t port;
    uint32_t bytesWritten;

    port = (int32_t)handle;

    bytesWritten = write(port, data, *size);

    if (bytesWritten != *size)
    {
        debugLog("Error from uart write: %s", strerror(errno));
        *size = 0;
        return FALSE;
    }

    *size = bytesWritten;

    return TRUE;
}
//--------------------------------------------------------------------------------------------------
bool_t platformUartRead(void* handle, uint8_t* buf, uint32_t* size)
{
    int32_t port;
    int32_t bytesRead;

    port = (int32_t)handle;

    bytesRead = read(port, buf, *size);

    if (bytesRead >= 0)
    {
        *size = bytesRead;
    }
    else
    {
        debugLog("Error from uart read: %s", strerror(errno));
        *size = 0;
    }

    return bytesRead >= 0;
}
//--------------------------------------------------------------------------------------------------
