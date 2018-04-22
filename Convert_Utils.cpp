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

int process_request(char *buf)
{
    char *primitiveSt = strstr(buf, PRIMITIVE[I_REQUEST]);
    if (primitiveSt != NULL)
        {
            char *portSt = strchr(primitiveSt, ' ');
            if (portSt != NULL)
                {
                    int port = atoi(portSt);
                    return port;
                }
            else
                return -1;
        }
    else
        return -1;
}

int process_response(char *buf)
{
    char *primitiveSt = strstr(buf, PRIMITIVE[I_RESPONSE]);
    if (primitiveSt != NULL)
        {
            char *portSt = strchr(primitiveSt, ' ');
            if (portSt != NULL)
                {
                    int port = atoi(portSt);
                    return port;
                }
            else
                return -1;
        }
    else
        return -1;
}

int process_disconnect(char *buf)
{
    char *primitiveSt = strstr(buf, PRIMITIVE[I_DISCONNECT]);
    if (primitiveSt != NULL)
        {
            char *portSt = strchr(primitiveSt, ' ');
            if (portSt != NULL)
                {
                    int port = atoi(portSt);
                    return port;
                }
            else
                return -1;
        }
    else
        return -1;
}