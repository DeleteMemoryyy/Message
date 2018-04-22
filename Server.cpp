#include "Def.h"

using namespace std;

#define MAX_QUEUE_SIZE (10)

int main()
{
    init_socket();

    SOCKET sockSrv = socket(AF_INET, SOCK_DGRAM, 0);
    unsigned long ul = 1;
    if (ioctlsocket(sockSrv, FIONBIO, (unsigned long *)&ul) == SOCKET_ERROR)
        {
            return 1;
        }
    SOCKET sockMsg = socket(AF_INET, SOCK_STREAM, 0);

#if defined(WIN32)
    SOCKADDR_IN addrSrv, addrMsg, addrConn;
    addrSrv.sin_addr.S_un.S_addr = htonl(INADDR_ANY);
#elif defined(__linux__)
    sockaddr_in addrSrv, addrMsg, addrConn;
    addrSrv.sin_addr.s_addr = htonl(INADDR_ANY);
#endif
    addrSrv.sin_port = htons(3614);
    addrSrv.sin_family = AF_INET;

    bind(sockSrv, (SOCKADDR *)&addrSrv, sizeof(SOCKADDR));

#if defined(WIN32)
    int addrLen = sizeof(SOCKADDR);
#elif defined(__linux__)
    unsigned int addrLen = sizeof(SOCKADDR);
#endif

    char srvBuf[100];
    char sendBuf[CONNECT_BUF_SIZE];
    char recvBuf[CONNECT_BUF_SIZE];
    int connPort = 4000;
    while (true)
        {
            int recvLen = recvfrom(sockSrv, srvBuf, sizeof(srvBuf), MSG_DONTWAIT,
                                   (SOCKADDR *)&addrMsg, &addrLen);

            if (recvLen > 0)
                {
                    connPort = process_request(srvBuf);
                    printf("Receive request for port: %d\n", connPort);
                    if (connPort < 0)
                        continue;

#if defined(WIN32)
                    memcpy(&addrConn, &addrMsg, sizeof(SOCKADDR_IN));
#elif defined(__linux__)
                    memcpy(&addrConn, &addrMsg, sizeof(sockaddr_in));
#endif
                    addrConn.sin_port = htons(connPort);

                    bind(sockMsg, (SOCKADDR *)&addrConn, sizeof(SOCKADDR));
                    listen(sockMsg, MAX_QUEUE_SIZE);

                    sprintf(srvBuf, "%s %d", PRIMITIVE[I_RESPONSE], connPort);
                    sendto(sockSrv, srvBuf, strlen(srvBuf) + 1, MSG_DONTWAIT, (SOCKADDR *)&addrMsg,
                           sizeof(SOCKADDR));

                    SOCKET sockConn = accept(sockMsg, (SOCKADDR *)&addrConn, &addrLen);
                    sprintf(sendBuf, "Connect established at port %d", connPort);
                    send(sockConn, sendBuf, strlen(sendBuf) + 1, MSG_NOSIGNAL);

                    while (true)
                        {
                            recvLen = recvfrom(sockSrv, srvBuf, sizeof(srvBuf), MSG_DONTWAIT,
                                               (SOCKADDR *)&addrMsg, &addrLen);
                            if (recvLen > 0 && process_disconnect(srvBuf) >= 0)
                                {
                                    sprintf(srvBuf, "%s", PRIMITIVE[I_RESPONSE_DISCONNECT]);
                                    sendto(sockSrv, srvBuf, strlen(srvBuf) + 1, MSG_DONTWAIT,
                                           (SOCKADDR *)&addrMsg, sizeof(SOCKADDR));
                                    break;
                                }

                            recv(sockConn, recvBuf, CONNECT_BUF_SIZE, 0);
                            printf("Received: %s\n", recvBuf);
                            sprintf(sendBuf, "Server Received: %s", recvBuf);
                            send(sockConn, sendBuf, strlen(sendBuf) + 1, MSG_NOSIGNAL);
                        }
#if defined(WIN32)
                    closesocket(sockConn);
#elif defined(__linux__)
                    close(sockConn);
#endif
                    break;
                }
        }

#if defined(WIN32)
    closesocket(sockMsg);
    closesocket(sockSrv);
    WSACleanup();
#elif defined(__linux__)
    close(sockMsg);
    close(sockSrv);
#endif

    return 0;
}