#include "Def.h"

using namespace std;

#define MAX_QUEUE_SIZE (10)

int main()
{
    init_socket();

    SOCKET sockSrv = socket(AF_INET, SOCK_STREAM, 0);
    SOCKET sockMsg = socket(AF_INET, SOCK_STREAM, 0);

#if defined(WIN32)
    SOCKADDR_IN addrSrv;
    addrSrv.sin_addr.S_un.S_addr = htonl(INADDR_ANY);
    addrSrv.sin_port = htons(3614);
    addrSrv.sin_family = AF_INET;
#elif defined(__linux__)
    sockaddr_in addrSrv;
    addrSrv.sin_addr.s_addr = htonl(INADDR_ANY);
    addrSrv.sin_port = htons(3614);
    addrSrv.sin_family = AF_INET;
#endif

    bind(sockSrv, (SOCKADDR *)&addrSrv, sizeof(SOCKADDR));

    SOCKADDR_IN addrClient;
    char srvBuf[100];

#if defined(WIN32)
    int addrLen = sizeof(SOCKADDR);
#elif defined(__linux__)
    unsigned int addrLen = sizeof(SOCKADDR);
#endif

    while (true)
        {
            int recvLen =
                recvfrom(sockSrv, srvBuf, sizeof(srvBuf), MSG_DONTWAIT, &addrSrv, &addrLen);

            // SOCKET sockConn = accept(sockSrv, (SOCKADDR *)&addrClient, &sockAddrLen);
            // listen(sockSrv, MAX_QUEUE_SIZE);

            char sendBuf[100];
            sprintf(sendBuf, "Welcome %s to the server program~ \nNow, let's start talking...\n",
                    inet_ntoa(addrClient.sin_addr));
            send(sockConn, sendBuf, strlen(sendBuf) + 1, MSG_NOSIGNAL);
            char recvBuf[100];
            for (int i = 0; i < 5; ++i)
                {
                    recv(sockConn, recvBuf, 100, 0);
                    printf("%s\n", recvBuf);
                    sprintf(sendBuf, "What you\'ve just sent: %s", recvBuf);
                    send(sockConn, sendBuf, strlen(sendBuf) + 1, MSG_NOSIGNAL);
                }

#if defined(WIN32)
            closesocket(sockConn);
#elif defined(__LINUX__)
            close(sockConn);
#endif
            break;
        }

#ifdef WIN32
    WSACleanup();
#endif

    return 0;
}