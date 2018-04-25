#ifndef UI_H_
#define UI_H_

#include "Def.h"
#include <GL/gl3w.h>
#include <GLFW/glfw3.h>
#include <cstdio>
#include <imgui.h>
#include <imgui_impl_glfw_gl3.h>
GLFWwindow *init_ui(GLFWerrorfun errFun);

void render(GLFWwindow *window);

void glfw_error_callback(int error, const char *description);

class Console
{
    char InputBuf[256];
    ImVector<char *> Items;
    bool ScrollToBottom;
    ImVector<char *> History;
    int HistoryPos;  // -1: new line, 0..History.Size-1 browsing history.
    ImVector<const char *> Commands;

    int Stricmp(const char *str1, const char *str2);
    int Strnicmp(const char *str1, const char *str2, int n);
    char *Strdup(const char *str);
    void ClearLog();
    void ExecCommand(const char *command_line);

  public:
    bool inputFlag;
    bool disconnectFlag;
    char sendBuf[CONNECT_BUF_SIZE];

    Console();
    ~Console();
    void AddLog(const char *fmt, ...) IM_FMTARGS(2);
    void Draw(const char *title, bool *p_open, char *ip, int port, unsigned long long time);
    int TextEditCallback(ImGuiTextEditCallbackData *data);
};

int TextEditCallbackStub(ImGuiTextEditCallbackData *data);

#endif
