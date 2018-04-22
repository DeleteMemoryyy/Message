#include "Def.h"

int init_socket()
{
#ifdef WIN32
    setlocale(LC_CTYPE, "");
    WORD wVersionRequested;
    WSADATA wsaData;
    int err = 0;

    wVersionRequested = MAKEWORD(1, 1);
    err = WSAStartup(wVersionRequested, &wsaData);
    if (err != 0 || LOBYTE(wsaData.wVersion) != 1 || HIBYTE(wsaData.wVersion) != 1)
        {
            WSACleanup();
            err = -1;
        }
    return err;
#endif
    return 0;
}