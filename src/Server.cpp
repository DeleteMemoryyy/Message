#include "Def.h"

#define MAX_QUEUE_SIZE (10)

int main()
{
    init_socket();

    SOCKET sockSrv = socket(AF_INET, SOCK_DGRAM, 0);
#if defined(WIN32)
    unsigned long ul = 1;
    if (ioctlsocket(sockSrv, FIONBIO, (unsigned long *)&ul) == SOCKET_ERROR)
        {
            return 1;
        }
#elif defined(__linux__)
    int flags = fcntl(sockSrv, F_GETFL, 0);
    if (fcntl(sockSrv, F_SETFL, flags | O_NONBLOCK) < 0)
        {
            return 1;
        }
#endif

#if defined(WIN32)
    SOCKADDR_IN addrSrv, addrMsg, addrConn;
    addrSrv.sin_addr.S_un.S_addr = htonl(INADDR_ANY);
#elif defined(__linux__)
    sockaddr_in addrSrv, addrMsg, addrConn;
    addrSrv.sin_addr.s_addr = htonl(INADDR_ANY);
#endif
    addrSrv.sin_port = htons(3614);
    addrSrv.sin_family = AF_INET;

    if (bind(sockSrv, (SOCKADDR *)&addrSrv, sizeof(SOCKADDR)) < 0)
        printf("srv socket bind faild.\n");

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
                    // struct in_addr addrTmp;
                    // unsigned long l1 = addrMsg.sin_addr.s_addr;
                    // memcpy(&addrTmp, &l1, 4);
                    printf("Receive request from %s:%d for port %d\n", inet_ntoa(addrMsg.sin_addr),
                           htons(addrMsg.sin_port), connPort);
                    if (connPort < 0)
                        continue;

                    addrSrv.sin_port = htons(connPort);
                    SOCKET sockMsg = socket(AF_INET, SOCK_STREAM, 0);

                    if (bind(sockMsg, (SOCKADDR *)&addrSrv, sizeof(SOCKADDR)) < 0)
                        {
                            printf("Bind faild.\n");
                        }

                    if (listen(sockMsg, MAX_QUEUE_SIZE) < 0)
                        {
                            printf("Listen faild.\n");
                        }

                    sprintf(srvBuf, "%s %d", PRIMITIVE[I_RESPONSE], connPort);
                    sendto(sockSrv, srvBuf, strlen(srvBuf) + 1, MSG_DONTWAIT, (SOCKADDR *)&addrMsg,
                           sizeof(SOCKADDR));

                    SOCKET sockConn = accept(sockMsg, (SOCKADDR *)&addrConn, &addrLen);
// #if defined(WIN32)
//                     ul = 1;
//                     if (ioctlsocket(sockConn, FIONBIO, (unsigned long *)&ul) == SOCKET_ERROR)
//                         {
//                             return 1;
//                         }
// #elif defined(__linux__)
//                     flags = fcntl(sockConn, F_GETFL, 0);
//                     if (fcntl(sockConn, F_SETFL, flags | O_NONBLOCK) < 0)
//                         {
//                             return 1;
//                         }
// #endif

                    sprintf(sendBuf, "Connect established at port %d", connPort);
                    send(sockConn, sendBuf, strlen(sendBuf) + 1, 0);
                    printf("Connect established at port %d\n", connPort);

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

#if defined(WIN32)
                            recv(sockConn, recvBuf, CONNECT_BUF_SIZE, 0);

                            // if (ret == SOCKET_ERROR)
                            //     {
                            //         int err = WSAGetLastError();
                            //         if (err == WSAEWOULDBLOCK)
                            //             {
                            //                 // continue;
                            //             }
                            //         else if (err == WSAETIMEDOUT)
                            //             {
                            //                 printf("TIMEOUT\n");
                            //                 break;
                            //             }
                            //         else if (err == WSAENOTCONN)
                            //             {
                            //                 printf("NOTCONN\n");
                            //                 continue;
                            //             }
                            //         else if (err == WSAENETDOWN)
                            //             {
                            //                 printf("NETDOWN\n");
                            //                 break;
                            //             }
                            //         else
                            //             break;
                            //     }
#elif defined(__linux__)
                            recv(sockConn, recvBuf, CONNECT_BUF_SIZE, 0);

                            // if (errno != EINPROGRESS)
                            //     {
                            //         int err = errno;
                            //         // printf("Errno: %d\n", err);
                            //         if (err == EINTR)
                            //             {
                            //                 printf("INTR\n");
                            //                 break;
                            //             }
                            //         // if (err == EPIPE)
                            //         //     break;
                            //         if (err == EAGAIN)
                            //             {
                            //                 // printf("AGAIN\n");
                            //                 // continue;
                            //             }
                            //         else
                            //             printf("ERR: %d\n", err);
                            //     }
#endif
                            // else
                                {
                                    printf("Received: %s\n", recvBuf);
                                    sprintf(sendBuf, "Server Received: %s", recvBuf);
                                    send(sockConn, sendBuf, strlen(sendBuf) + 1, 0);
                                }
                        }

                    printf("Client has disconnected.\n");
#if defined(WIN32)
                    closesocket(sockConn);
                    closesocket(sockMsg);
#elif defined(__linux__)
                    close(sockConn);
                    close(sockMsg);
#endif
                }
        }

#if defined(WIN32)
    closesocket(sockSrv);
    WSACleanup();
#elif defined(__linux__)
    close(sockSrv);
#endif

    return 0;
}