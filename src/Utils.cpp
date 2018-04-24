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

unsigned long long GetCurrentTimeMsec()
{
#if defined(WIN32)
    struct timeval tv;
    time_t clock;
    struct tm tm;
    SYSTEMTIME wtm;

    GetLocalTime(&wtm);
    tm.tm_year = wtm.wYear - 1900;
    tm.tm_mon = wtm.wMonth - 1;
    tm.tm_mday = wtm.wDay;
    tm.tm_hour = wtm.wHour;
    tm.tm_min = wtm.wMinute;
    tm.tm_sec = wtm.wSecond;
    tm.tm_isdst = -1;
    clock = mktime(&tm);
    tv.tv_sec = clock;
    tv.tv_usec = wtm.wMilliseconds * 1000;
    return ((unsigned long long)tv.tv_sec * 1000 + (unsigned long long)tv.tv_usec / 1000);
#elif defined(__linux__)
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return ((unsigned long long)tv.tv_sec * 1000 + (unsigned long long)tv.tv_usec / 1000);
#endif
    return 0;
}