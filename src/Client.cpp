#include "Def.h"
#include "UI.h"

using namespace std;

int main()
{
    // Init UI
    GLFWwindow *window = init_ui(glfw_error_callback);
    if (window == NULL)
        exit(1);

    // Init socket
    init_socket();
    SOCKET sockCli = socket(AF_INET, SOCK_DGRAM, 0);

#if defined(WIN32)
    unsigned long ul = 1;
    if (ioctlsocket(sockCli, FIONBIO, (unsigned long *)&ul) == SOCKET_ERROR)
        {
            return 1;
        }
#elif defined(__linux__)
    int flags = fcntl(sockCli, F_GETFL, 0);
    if (fcntl(sockCli, F_SETFL, flags | O_NONBLOCK) < 0)
        {
            return 1;
        }
#endif

    int connIp[4] = {47, 106, 157, 25};
    int connPort = 4001;

    while (!glfwWindowShouldClose(window))
        {
            SOCKET sockConn = socket(AF_INET, SOCK_STREAM, 0);
#if defined(WIN32)
            if (ioctlsocket(sockConn, FIONBIO, (unsigned long *)&ul) == SOCKET_ERROR)
                {
                    return 1;
                }
#elif defined(__linux__)
            flags = fcntl(sockConn, F_GETFL, 0);
            if (fcntl(sockConn, F_SETFL, flags | O_NONBLOCK) < 0)
                {
                    return 1;
                }
#endif

            bool configFinish = false, configError = false;
            while (!glfwWindowShouldClose(window))
                {
                    glfwPollEvents();
                    ImGui_ImplGlfwGL3_NewFrame();

                    ImGui::SetNextWindowSize(ImVec2(450, 200), ImGuiSetCond_Once);
                    ImGui::SetNextWindowPos(ImVec2(40, 20), ImGuiSetCond_Once);
                    ImGui::Begin("Config", NULL,
                                 ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
                                     ImGuiWindowFlags_NoCollapse);

                    ImGui::Text("Configure host address and port");

                    ImGui::InputInt4(" IP ", connIp);

                    ImGui::InputInt(" Port (4000 - 6000) ", &connPort);

                    if (ImGui::Button("Connect"))
                        {
                            if (!(connIp[0] >= 0 && connIp[0] <= 255 && connIp[1] >= 0 &&
                                  connIp[1] <= 255 && connIp[2] >= 0 && connIp[2] <= 255 &&
                                  connIp[3] >= 0 && connIp[3] <= 255 && connPort >= 4000 &&
                                  connPort <= 6000))
                                configError = true;
                            else
                                configFinish = true;
                        }

                    if (configError)
                        ImGui::Text("Config error. Please check.");

                    ImGui::End();

                    // Rendering
                    render(window);
                    if (configFinish)
                        break;
                }
            if (glfwWindowShouldClose(window))
                exit(1);

            char strConnIp[100];
            sprintf(strConnIp, "%d.%d.%d.%d", connIp[0], connIp[1], connIp[2], connIp[3]);

#if defined(WIN32)
            SOCKADDR_IN addrCli, addrMsg, addrConn;
            addrCli.sin_addr.S_un.S_addr = inet_addr(strConnIp);
            addrCli.sin_family = AF_INET;
            addrCli.sin_port = htons(3614);
            addrConn.sin_family = AF_INET;
            addrConn.sin_port = htons(connPort);
            addrConn.sin_addr.s_addr = inet_addr(strConnIp);
#elif defined(__linux__)
            sockaddr_in addrCli, addrMsg, addrConn;
            addrCli.sin_family = AF_INET;
            addrCli.sin_port = htons(3614);
            addrCli.sin_addr.s_addr = inet_addr(strConnIp);
            addrConn.sin_family = AF_INET;
            addrConn.sin_port = htons(connPort);
            addrConn.sin_addr.s_addr = inet_addr(strConnIp);
#endif

#if defined(WIN32)
            int addrLen = sizeof(SOCKADDR);
#elif defined(__linux__)
            unsigned int addrLen = sizeof(SOCKADDR);
#endif

            char cliBuf[100];
            char recvBuf[CONNECT_BUF_SIZE];

            bind(sockCli, (SOCKADDR *)&addrCli, sizeof(SOCKADDR));

            sprintf(cliBuf, "%s %d", PRIMITIVE[I_REQUEST], connPort);
            printf("Send: %s\n", cliBuf);
            sendto(sockCli, cliBuf, strlen(cliBuf) + 1, MSG_DONTWAIT, (SOCKADDR *)&addrCli,
                   sizeof(SOCKADDR));
            while (true)
                {
                    int recvLen = recvfrom(sockCli, cliBuf, sizeof(cliBuf), MSG_DONTWAIT,
                                           (SOCKADDR *)&addrMsg, &addrLen);
                    if (recvLen > 0)
                        {
                            connPort = process_response(cliBuf);
                            // struct in_addr addrTmp;
                            // unsigned long l1 = addrMsg.sin_addr.s_addr;
                            // memcpy(&addrTmp, &l1, 4);
                            printf("Recieve response from %s:%d for port %d\n",
                                   inet_ntoa(addrMsg.sin_addr), htons(addrMsg.sin_port), connPort);
                            if (connPort < 0)
                                continue;

                            // l1 = addrConn.sin_addr.s_addr;
                            // memcpy(&addrTmp, &l1, 4);
                            printf("Connecting %s:%d\n", inet_ntoa(addrConn.sin_addr),
                                   htons(addrConn.sin_port));
                            addrConn.sin_port = htons(connPort);
                            unsigned long long timeSent = GetCurrentTimeMsec();
                            unsigned long long timeLag = -1LL;
                            bool clacLag = true;
                            connect(sockConn, (SOCKADDR *)&addrConn, sizeof(SOCKADDR));
                            char *strIp = inet_ntoa(addrConn.sin_addr);
                            printf("Connect to server %s:%d\n", strIp, connPort);

                            Console console;

                            int connFlag = true;
                            bool consoleOpen = true;
                            while (true)
                                {
                                    int ret =
                                        recv(sockConn, recvBuf, CONNECT_BUF_SIZE, MSG_DONTWAIT);
#if defined(WIN32)
                                    if (ret == SOCKET_ERROR)
                                        {
                                            int err = WSAGetLastError();
                                            if (err == WSAEWOULDBLOCK)
                                                {
                                                    // continue;
                                                }
                                            else if (err == WSAETIMEDOUT)
                                                {
                                                    printf("TIMEOUT\n");
                                                    break;
                                                }
                                            else if (err == WSAENOTCONN)
                                                {
                                                    printf("NOTCONN\n");
                                                    continue;
                                                }
                                            else if (err == WSAENETDOWN)
                                                {
                                                    printf("SHUTDOWN\n");
                                                    break;
                                                }
                                            else
                                                {
                                                    printf("%d\n", err);
                                                    break;
                                                }
                                        }
#elif defined(__linux__)
                                    if (ret < 0)
                                        {
                                            int err = errno;
                                            if (err == EAGAIN)
                                                {
                                                    // printf("AGAIN\n");
                                                    // continue;
                                                }
                                            else if (err == EINTR)
                                                {
                                                    printf("INTR\n");
                                                    // continue;
                                                }
                                            else if (err == ETIMEDOUT)
                                                {
                                                    printf("TIMEOUT\n");
                                                    break;
                                                }
                                            else if (err == EWOULDBLOCK)
                                                {
                                                    printf("WOULDBLOCKWO");
                                                    // continue;
                                                }
                                            else
                                                {
                                                    printf("%d\n", err);
                                                }
                                        }
#endif
                                    else
                                        {
                                            if (clacLag)
                                                {
                                                    timeLag = (GetCurrentTimeMsec() - timeSent) / 2;
                                                    clacLag = false;
                                                }
                                            console.AddLog("[Server] %s\n\n", recvBuf);
                                            printf("Redeived: %s\n", recvBuf);
                                        }

                                    // printf("Redeived: %s\n", recvBuf);
                                    // printf("Enter message to send:\n");
                                    // scanf("%s", sendBuf);
                                    glfwPollEvents();
                                    ImGui_ImplGlfwGL3_NewFrame();

                                    console.Draw("Message", &consoleOpen, strIp, connPort, timeLag);
                                    if (console.inputFlag)
                                        {
                                            // printf("[Message received] %s\n", recvBuf);
                                            printf("[Message to send] %s\n", console.sendBuf);
                                            timeSent = GetCurrentTimeMsec();
                                            clacLag = true;
                                            send(sockConn, console.sendBuf,
                                                 strlen(console.sendBuf) + 1, MSG_DONTWAIT);
                                        }
                                    if (glfwWindowShouldClose(window) || console.disconnectFlag)
                                        connFlag = false;

                                    if (!connFlag)
                                        {
                                            sprintf(cliBuf, "%s %d", PRIMITIVE[I_DISCONNECT],
                                                    connPort);
                                            sendto(sockCli, cliBuf, strlen(cliBuf) + 1,
                                                   MSG_DONTWAIT, (SOCKADDR *)&addrCli,
                                                   sizeof(SOCKADDR));
                                            render(window);

                                            break;
                                        }
                                    render(window);
                                }

#if defined(WIN32)
                            closesocket(sockConn);
#elif defined(__linux__)
                            close(sockConn);
#endif
                            break;
                        }
                }
        }

#if defined(WIN32)
    closesocket(sockCli);
    WSACleanup();
#elif defined(__linux__)
    close(sockCli);
#endif

    // Cleanup UI
    ImGui_ImplGlfwGL3_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}
