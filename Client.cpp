#include "Def.h"

using namespace std;

int main()
{
    init_socket();

    SOCKET sockClient = socket(AF_INET, SOCK_STREAM, 0);

#if defined(WIN32)
    SOCKADDR_IN addrSrv;
    addrSrv.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");
    addrSrv.sin_family = AF_INET;
    addrSrv.sin_port = htons(3614);
#elif defined(__linux__)
    sockaddr_in addrSrv;
    addrSrv.sin_family = AF_INET;
    addrSrv.sin_port = htons(3614);
    addrSrv.sin_addr.s_addr = inet_addr("127.0.0.1");
#endif

    connect(sockClient, (SOCKADDR *)&addrSrv, sizeof(SOCKADDR));

    char recvBuf[100];
    recv(sockClient, recvBuf, 100, 0);
    printf("%s\n", recvBuf);

    send(sockClient, "Attention: A Client has enter...\n",
         strlen("Attention: A Client has enter...\n") + 1, MSG_NOSIGNAL);

    int n = 5;
    do
        {
            char talk[100];
            printf("\nPlease enter what you want to say next(\"quit\"to exit):");
            scanf("%s", talk);
            send(sockClient, talk, strlen(talk) + 1, MSG_NOSIGNAL);

            char recvBuf[100];
            recv(sockClient, recvBuf, 100, 0);
            printf("%s", recvBuf);
        }
    while (--n);

#if defined(WIN32)
    closesocket(sockClient);
    WSACleanup();
#elif defined(__LINUX__)
    close(sockClient);
#endif

    return 0;
}
