#include "Def.h"
#include "UI.h"

using namespace std;

void render(GLFWwindow *window, ImVec4 &clear_color)
{
    int display_w, display_h;
    glfwGetFramebufferSize(window, &display_w, &display_h);
    glViewport(0, 0, display_w, display_h);
    glClearColor(clear_color.x, clear_color.y, clear_color.z, clear_color.w);
    glClear(GL_COLOR_BUFFER_BIT);
    ImGui::Render();
    ImGui_ImplGlfwGL3_RenderDrawData(ImGui::GetDrawData());
    glfwSwapBuffers(window);
}

struct Console
{
    char InputBuf[256];
    char sendBuf[CONNECT_BUF_SIZE];
    ImVector<char *> Items;
    bool ScrollToBottom;
    ImVector<char *> History;
    int HistoryPos;  // -1: new line, 0..History.Size-1 browsing history.
    bool inputFlag;
    bool disconnectFlag;
    ImVector<const char *> Commands;

    Console()
    {
        ClearLog();
        memset(InputBuf, 0, sizeof(InputBuf));
        HistoryPos = -1;
        Commands.push_back("HELP");
        Commands.push_back("HISTORY");
        Commands.push_back("CLEAR");
        Commands.push_back("CLASSIFY");  // "classify" is here to provide an example of "C"+[tab]
                                         // completing to "CL" and displaying matches.
        AddLog("Connect success!\n\n");
    }
    ~Console()
    {
        ClearLog();
        for (int i = 0; i < History.Size; i++)
            free(History[i]);
    }

    // Portable helpers
    static int Stricmp(const char *str1, const char *str2)
    {
        int d;
        while ((d = toupper(*str2) - toupper(*str1)) == 0 && *str1)
            {
                str1++;
                str2++;
            }
        return d;
    }
    static int Strnicmp(const char *str1, const char *str2, int n)
    {
        int d = 0;
        while (n > 0 && (d = toupper(*str2) - toupper(*str1)) == 0 && *str1)
            {
                str1++;
                str2++;
                n--;
            }
        return d;
    }
    static char *Strdup(const char *str)
    {
        size_t len = strlen(str) + 1;
        void *buff = malloc(len);
        return (char *)memcpy(buff, (const void *)str, len);
    }

    void ClearLog()
    {
        for (int i = 0; i < Items.Size; i++)
            free(Items[i]);
        Items.clear();
        ScrollToBottom = true;
    }

    void AddLog(const char *fmt, ...) IM_FMTARGS(2)
    {
        // FIXME-OPT
        char buf[1024];
        va_list args;
        va_start(args, fmt);
        vsnprintf(buf, IM_ARRAYSIZE(buf), fmt, args);
        buf[IM_ARRAYSIZE(buf) - 1] = 0;
        va_end(args);
        Items.push_back(Strdup(buf));
        ScrollToBottom = true;
    }

    void Draw(const char *title, bool *p_open, char *ip, int port)
    {
        ImGui::SetNextWindowSize(ImVec2(580, 600), ImGuiSetCond_Always);
        if (!ImGui::Begin(title, p_open,
                          ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
                              ImGuiWindowFlags_NoCollapse))
            {
                ImGui::End();
                return;
            }

        // if (ImGui::BeginPopupContextItem())
        //     {
        //         if (ImGui::MenuItem("Close"))
        //             *p_open = false;
        //         ImGui::EndPopup();
        //     }

        ImGui::Text("Connect to server %s:%d\n", ip, port);

        ImGui::SameLine();
        if (ImGui::SmallButton("Clear"))
            {
                ClearLog();
            }
        ImGui::SameLine();
        bool copy_to_clipboard = ImGui::SmallButton("Copy");
        ImGui::SameLine();
        if (ImGui::SmallButton("Scroll to bottom"))
            ScrollToBottom = true;
        ImGui::SameLine();
        disconnectFlag = false;
        if (ImGui::SmallButton("Disconnect"))
            disconnectFlag = true;
        // static float t = 0.0f; if (ImGui::GetTime() - t > 0.02f) { t = ImGui::GetTime();
        // AddLog("Spam %f", t); }

        ImGui::Separator();

        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 0));
        static ImGuiTextFilter filter;
        filter.Draw("Filter", 180);
        ImGui::PopStyleVar();
        ImGui::Separator();

        const float footer_height_to_reserve =
            ImGui::GetStyle().ItemSpacing.y +
            ImGui::GetFrameHeightWithSpacing();  // 1 separator, 1 input text
        ImGui::BeginChild(
            "ScrollingRegion", ImVec2(0, -footer_height_to_reserve), false,
            ImGuiWindowFlags_HorizontalScrollbar);  // Leave room for 1 separator + 1 InputText
        if (ImGui::BeginPopupContextWindow())
            {
                if (ImGui::Selectable("Clear"))
                    ClearLog();
                ImGui::EndPopup();
            }

        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(4, 1));  // Tighten spacing
        if (copy_to_clipboard)
            ImGui::LogToClipboard();
        ImVec4 col_default_text = ImGui::GetStyleColorVec4(ImGuiCol_Text);
        for (int i = 0; i < Items.Size; i++)
            {
                const char *item = Items[i];
                if (!filter.PassFilter(item))
                    continue;
                ImVec4 col = col_default_text;
                if (strstr(item, "[Server]"))
                    col = ImColor(1.0f, 0.4f, 0.4f, 1.0f);
                else if (strncmp(item, "# ", 2) == 0)
                    col = ImColor(1.0f, 0.78f, 0.58f, 1.0f);
                ImGui::PushStyleColor(ImGuiCol_Text, col);
                ImGui::TextUnformatted(item);
                ImGui::PopStyleColor();
            }
        if (copy_to_clipboard)
            ImGui::LogFinish();
        if (ScrollToBottom)
            ImGui::SetScrollHere();
        ScrollToBottom = false;
        ImGui::PopStyleVar();
        ImGui::EndChild();
        ImGui::Separator();

        // Command-line
        bool reclaim_focus = false;
        inputFlag = false;
        if (ImGui::InputText("Input", InputBuf, IM_ARRAYSIZE(InputBuf),
                             ImGuiInputTextFlags_EnterReturnsTrue |
                                 ImGuiInputTextFlags_CallbackCompletion |
                                 ImGuiInputTextFlags_CallbackHistory,
                             &TextEditCallbackStub, (void *)this))
            {
                char *input_end = InputBuf + strlen(InputBuf);
                while (input_end > InputBuf && input_end[-1] == ' ')
                    {
                        input_end--;
                    }
                *input_end = 0;
                if (InputBuf[0])
                    ExecCommand(InputBuf);
                strcpy(sendBuf, InputBuf);
                strcpy(InputBuf, "");
                reclaim_focus = true;
            }

        // Demonstrate keeping focus on the input box
        ImGui::SetItemDefaultFocus();
        if (reclaim_focus)
            ImGui::SetKeyboardFocusHere(-1);  // Auto focus previous widget

        ImGui::End();
    }

    void ExecCommand(const char *command_line)
    {
        AddLog("# Client\n");

        HistoryPos = -1;
        for (int i = History.Size - 1; i >= 0; i--)
            if (Stricmp(History[i], command_line) == 0)
                {
                    free(History[i]);
                    History.erase(History.begin() + i);
                    break;
                }
        History.push_back(Strdup(command_line));

        // Process command
        if (Stricmp(command_line, "CLEAR") == 0)
            {
                ClearLog();
            }
        else if (Stricmp(command_line, "HELP") == 0)
            {
                AddLog("Commands:");
                for (int i = 0; i < Commands.Size; i++)
                    AddLog("- %s", Commands[i]);
            }
        else if (Stricmp(command_line, "HISTORY") == 0)
            {
                int first = History.Size - 10;
                for (int i = first > 0 ? first : 0; i < History.Size; i++)
                    AddLog("%3d: %s\n", i, History[i]);
            }
        else
            {
                inputFlag = true;
                AddLog("%s\n\n", command_line);
            }
    }

    static int TextEditCallbackStub(ImGuiTextEditCallbackData *data)
    {
        Console *console = (Console *)data->UserData;
        return console->TextEditCallback(data);
    }

    int TextEditCallback(ImGuiTextEditCallbackData *data)
    {
        // AddLog("cursor: %d, selection: %d-%d", data->CursorPos, data->SelectionStart,
        // data->SelectionEnd);
        switch (data->EventFlag)
            {
                case ImGuiInputTextFlags_CallbackCompletion:
                    {
                        // Example of TEXT COMPLETION

                        // Locate beginning of current word
                        const char *word_end = data->Buf + data->CursorPos;
                        const char *word_start = word_end;
                        while (word_start > data->Buf)
                            {
                                const char c = word_start[-1];
                                if (c == ' ' || c == '\t' || c == ',' || c == ';')
                                    break;
                                word_start--;
                            }

                        // Build a list of candidates
                        ImVector<const char *> candidates;
                        for (int i = 0; i < Commands.Size; i++)
                            if (Strnicmp(Commands[i], word_start, (int)(word_end - word_start)) ==
                                0)
                                candidates.push_back(Commands[i]);

                        if (candidates.Size == 0)
                            {
                                // No match
                                AddLog("No match for \"%.*s\"!\n", (int)(word_end - word_start),
                                       word_start);
                            }
                        else if (candidates.Size == 1)
                            {
                                // Single match. Delete the beginning of the word and replace it
                                // entirely so we've got nice casing
                                data->DeleteChars((int)(word_start - data->Buf),
                                                  (int)(word_end - word_start));
                                data->InsertChars(data->CursorPos, candidates[0]);
                                data->InsertChars(data->CursorPos, " ");
                            }
                        else
                            {
                                // Multiple matches. Complete as much as we can, so inputing "C"
                                // will complete to "CL" and display "CLEAR" and "CLASSIFY"
                                int match_len = (int)(word_end - word_start);
                                for (;;)
                                    {
                                        int c = 0;
                                        bool all_candidates_matches = true;
                                        for (int i = 0;
                                             i < candidates.Size && all_candidates_matches; i++)
                                            if (i == 0)
                                                c = toupper(candidates[i][match_len]);
                                            else if (c == 0 ||
                                                     c != toupper(candidates[i][match_len]))
                                                all_candidates_matches = false;
                                        if (!all_candidates_matches)
                                            break;
                                        match_len++;
                                    }

                                if (match_len > 0)
                                    {
                                        data->DeleteChars((int)(word_start - data->Buf),
                                                          (int)(word_end - word_start));
                                        data->InsertChars(data->CursorPos, candidates[0],
                                                          candidates[0] + match_len);
                                    }

                                // List matches
                                AddLog("Possible matches:\n");
                                for (int i = 0; i < candidates.Size; i++)
                                    AddLog("- %s\n", candidates[i]);
                            }

                        break;
                    }
                case ImGuiInputTextFlags_CallbackHistory:
                    {
                        // Example of HISTORY
                        const int prev_history_pos = HistoryPos;
                        if (data->EventKey == ImGuiKey_UpArrow)
                            {
                                if (HistoryPos == -1)
                                    HistoryPos = History.Size - 1;
                                else if (HistoryPos > 0)
                                    HistoryPos--;
                            }
                        else if (data->EventKey == ImGuiKey_DownArrow)
                            {
                                if (HistoryPos != -1)
                                    if (++HistoryPos >= History.Size)
                                        HistoryPos = -1;
                            }

                        // A better implementation would preserve the data on the current input line
                        // along with cursor position.
                        if (prev_history_pos != HistoryPos)
                            {
                                data->CursorPos = data->SelectionStart = data->SelectionEnd =
                                    data->BufTextLen =
                                        (int)snprintf(data->Buf, (size_t)data->BufSize, "%s",
                                                      (HistoryPos >= 0) ? History[HistoryPos] : "");
                                data->BufDirty = true;
                            }
                    }
            }
        return 0;
    }
};

static void glfw_error_callback(int error, const char *description)
{
    fprintf(stderr, "Error %d: %s\n", error, description);
}

int main()
{
    // Init Socket
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

    // Init UI
    // Setup window
    glfwSetErrorCallback(glfw_error_callback);
    if (!glfwInit())
        return 1;
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow *window = glfwCreateWindow(1280, 720, "Message Client", NULL, NULL);
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);  // Enable vsync
    gl3wInit();

    // Setup ImGui binding
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;  // Enable Keyboard Controls

    ImGui_ImplGlfwGL3_Init(window, true);

    // Setup style
    // ImGui::StyleColorsDark();
    // ImGui::StyleColorsClassic();
    ImGui::StyleColorsLight();
    io.Fonts->AddFontDefault();
    ImFont *font = io.Fonts->AddFontFromFileTTF("UI_LIB/extra_fonts/Yahei_Segoe.ttf", 16.0f, NULL,
                                                io.Fonts->GetGlyphRangesChinese());
    IM_ASSERT(font != NULL);
    io.FontDefault = font;

    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

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
                    ImGui::SetNextWindowPos(ImVec2(20, 5), ImGuiSetCond_Once);
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
                    render(window, clear_color);
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
                            connect(sockConn, (SOCKADDR *)&addrConn, sizeof(SOCKADDR));
                            char *strIp = inet_ntoa(addrConn.sin_addr);
                            printf("Connect to server %s:%d\n", strIp, connPort);

                            struct Console console;

                            int connFlag = true;
                            bool consoleOpen = true;
                            while (true)
                                {
#if defined(WIN32)
                                    int ret =
                                        recv(sockConn, recvBuf, CONNECT_BUF_SIZE, MSG_DONTWAIT);

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
                                    recv(sockConn, recvBuf, CONNECT_BUF_SIZE, MSG_DONTWAIT);

                                    int err = errno;
                                    if (err != EINPROGRESS)
                                        {
                                            if (err == EAGAIN)
                                                {
                                                    // continue;
                                                }
                                            else if (err == EINTR)
                                                {
                                                    printf("INTR\n");
                                                    break;
                                                }
                                            else
                                                {
                                                    printf("%d\n", err);
                                                }
                                        }
#endif
                                    else
                                        {
                                            console.AddLog("[Server] %s\n\n", recvBuf);
                                            printf("Redeived: %s\n", recvBuf);
                                        }

                                    // console.AddLog("[Server] %s", recvBuf);
                                    // printf("Redeived: %s\n", recvBuf);
                                    // printf("Enter message to send:\n");
                                    // scanf("%s", sendBuf);
                                    glfwPollEvents();
                                    ImGui_ImplGlfwGL3_NewFrame();

                                    console.Draw("Message", &consoleOpen, strIp, connPort);
                                    if (console.inputFlag)
                                        {
                                            printf("Message to send: %s\n", console.sendBuf);
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
                                            render(window, clear_color);

                                            break;
                                        }
                                    render(window, clear_color);
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
