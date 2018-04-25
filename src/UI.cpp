#include "UI.h"

ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

void render(GLFWwindow *window)
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

void glfw_error_callback(int error, const char *description)
{
    fprintf(stderr, "Error %d: %s\n", error, description);
}

GLFWwindow *init_ui(GLFWerrorfun errFun)
{
    // Setup window
    glfwSetErrorCallback(errFun);
    if (!glfwInit())
        return NULL;
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow *window = glfwCreateWindow(640, 720, "Message Client", NULL, NULL);
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

    // Setup font
    char path[256];
    char *ch;
#if defined(WIN32)
    GetModuleFileName(NULL, path, 256);
    ch = strrchr(path, '\\');
#elif defined(__linux__)
    readlink("/proc/self/exe", path, 256);
    ch = strrchr(path, '/');
#endif
    if (ch != NULL)
        {
            sprintf(ch + 1, "%s", "fonts/Yahei_Segoe.ttf");
            ImFont *font =
                io.Fonts->AddFontFromFileTTF(path, 16.0f, NULL, io.Fonts->GetGlyphRangesChinese());
            IM_ASSERT(font != NULL);
            io.FontDefault = font;
        }
    else
        return NULL;

    return window;
}

// Console methods
Console::Console()
{
    ClearLog();
    memset(InputBuf, 0, sizeof(InputBuf));
    HistoryPos = -1;
    Commands.push_back("HELP");
    Commands.push_back("HISTORY");
    Commands.push_back("CLEAR");

    AddLog("Connect success!\n\n");
}

Console::~Console()
{
    ClearLog();
    for (int i = 0; i < History.Size; i++)
        free(History[i]);
}

// Portable helpers
int Console::Stricmp(const char *str1, const char *str2)
{
    int d;
    while ((d = toupper(*str2) - toupper(*str1)) == 0 && *str1)
        {
            str1++;
            str2++;
        }
    return d;
}
int Console::Strnicmp(const char *str1, const char *str2, int n)
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
char *Console::Strdup(const char *str)
{
    size_t len = strlen(str) + 1;
    void *buff = malloc(len);
    return (char *)memcpy(buff, (const void *)str, len);
}

void Console::ClearLog()
{
    for (int i = 0; i < Items.Size; i++)
        free(Items[i]);
    Items.clear();
    ScrollToBottom = true;
}

void Console::AddLog(const char *fmt, ...)
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

void Console::Draw(const char *title, bool *p_open, char *ip, int port, unsigned long long time)
{
    ImGui::SetNextWindowSize(ImVec2(580, 600), ImGuiSetCond_Always);
    ImGui::SetNextWindowPos(ImVec2(30, 60), ImGuiSetCond_Always);
    if (!ImGui::Begin(title, NULL,
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
    ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor(252, 146, 114, 220));
    if (ImGui::SmallButton("Disconnect"))
        disconnectFlag = true;
    ImGui::PopStyleColor();
    // static float t = 0.0f; if (ImGui::GetTime() - t > 0.02f) { t = ImGui::GetTime();
    // AddLog("Spam %f", t); }

    ImGui::Separator();

    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 0));
    static ImGuiTextFilter filter;
    filter.Draw("Filter", 180);
    ImGui::PopStyleVar();
    ImGui::SameLine(500.0f);
    ImGui::Text("Lag: %ums", (unsigned int)time);
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

void Console::ExecCommand(const char *command_line)
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
            AddLog("\n");
        }
    else if (Stricmp(command_line, "HISTORY") == 0)
        {
            int first = History.Size - 10;
            for (int i = first > 0 ? first : 0; i < History.Size; i++)
                AddLog("%3d: %s\n", i, History[i]);
            AddLog("\n");
        }
    else
        {
            inputFlag = true;
            AddLog("%s\n\n", command_line);
        }
}

int TextEditCallbackStub(ImGuiTextEditCallbackData *data)
{
    Console *console = (Console *)data->UserData;
    return console->TextEditCallback(data);
}

int Console::TextEditCallback(ImGuiTextEditCallbackData *data)
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
                        if (Strnicmp(Commands[i], word_start, (int)(word_end - word_start)) == 0)
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
                            int match_len = (int)(word_end - word_start);
                            for (;;)
                                {
                                    int c = 0;
                                    bool all_candidates_matches = true;
                                    for (int i = 0; i < candidates.Size && all_candidates_matches;
                                         i++)
                                        if (i == 0)
                                            c = toupper(candidates[i][match_len]);
                                        else if (c == 0 || c != toupper(candidates[i][match_len]))
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