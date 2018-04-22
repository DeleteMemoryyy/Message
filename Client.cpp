#include "Def.h"
#include "UI.h"

using namespace std;

int main()
{
    init_socket();

    SOCKET sockCli = socket(AF_INET, SOCK_DGRAM, 0);
    unsigned long ul = 1;
    if (ioctlsocket(sockCli, FIONBIO, (unsigned long *)&ul) == SOCKET_ERROR)
        {
            return 1;
        }
    SOCKET sockConn = socket(AF_INET, SOCK_STREAM, 0);

#if defined(WIN32)
    SOCKADDR_IN addrCli, addrConn;
    addrCli.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");
    addrCli.sin_family = AF_INET;
    addrCli.sin_port = htons(3614);
#elif defined(__linux__)
    sockaddr_in addrCli, addrConn;
    addrCli.sin_family = AF_INET;
    addrCli.sin_port = htons(3614);
    addrCli.sin_addr.s_addr = inet_addr("127.0.0.1");
#endif

#if defined(WIN32)
    int addrLen = sizeof(SOCKADDR);
#elif defined(__linux__)
    unsigned int addrLen = sizeof(SOCKADDR);
#endif

    char cliBuf[100];
    char sendBuf[CONNECT_BUF_SIZE];
    char recvBuf[CONNECT_BUF_SIZE];
    int connPort = 4001;

    bind(sockCli, (SOCKADDR *)&addrCli, sizeof(SOCKADDR));

    sprintf(cliBuf, "%s %d", PRIMITIVE[I_REQUEST], connPort);
    printf("Send: %s\n", cliBuf);
    sendto(sockCli, cliBuf, strlen(cliBuf) + 1, MSG_DONTWAIT, (SOCKADDR *)&addrCli,
           sizeof(SOCKADDR));
    while (true)
        {
            int recvLen = recvfrom(sockCli, cliBuf, sizeof(cliBuf), MSG_DONTWAIT,
                                   (SOCKADDR *)&addrConn, &addrLen);
            if (recvLen > 0)
                {
                    connPort = process_response(cliBuf);
                    if (connPort < 0)
                        continue;
                    addrConn.sin_port = htons(connPort);
                    connect(sockConn, (SOCKADDR *)&addrConn, sizeof(SOCKADDR));
                    printf("Connect to server %s:%d\n", inet_ntoa(addrConn.sin_addr), connPort);

                    int connFlag = true;
                    while (true)
                        {
                            if (!connFlag)
                                {
                                    sprintf(cliBuf, "%s %d", PRIMITIVE[I_DISCONNECT], connPort);
                                    sendto(sockCli, cliBuf, strlen(cliBuf) + 1, MSG_DONTWAIT,
                                           (SOCKADDR *)&addrCli, sizeof(SOCKADDR));
                                    break;
                                }
                            recv(sockConn, recvBuf, CONNECT_BUF_SIZE, 0);
                            printf("Redeived: %s\n", recvBuf);
                            printf("Enter message to send:\n");
                            scanf("%s", sendBuf);
                            printf("Message to send: %s\n", sendBuf);
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
    closesocket(sockCli);
    WSACleanup();
#elif defined(__linux__)
    close(sockCli);
#endif

    return 0;
}
