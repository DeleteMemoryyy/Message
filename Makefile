CXX = g++
CC = gcc
MAKE = make
BINDIR = bin/
SERVER = Message_Server
CLIENT = Message_Client
UI_TEST = UI_Test
RM = rm

SERVER_EXE = $(BINDIR)$(SERVER)
CLIENT_EXE = $(BINDIR)$(CLIENT)
UI_TEST_EXE = $(BINDIR)$(UI_TEST)

SOURCES =
SERVER_SOURCES = Server.cpp
CLIENT_SOURCES = Client.cpp
UTILS_SOURCES = Utils.cpp
IMGUI_SOURCES = UI_LIB/imgui_impl_glfw_gl3.cpp UI_LIB/imgui.cpp UI_LIB/imgui_demo.cpp UI_LIB/imgui_draw.cpp
UI_TEST_SOURCES = UI_Test.cpp

OBJS = $(addsuffix .o, $(basename $(SOURCES)))
SERVER_OBJS = $(addsuffix .o, $(basename $(SERVER_SOURCES)))
CLIENT_OBJS = $(addsuffix .o, $(basename $(CLIENT_SOURCES)))
UTILS_OBJS = $(addsuffix .o, $(basename $(UTILS_SOURCES)))
IMGUI_OBJS = $(addsuffix .o, $(basename $(IMGUI_SOURCES)))
IMGUI_OBJS += UI_LIB/gl3w.o
IMGUI_NOTDIR_OBJS = $(notdir $(IMGUI_OBJS))
UI_TEST_OBJS = $(addsuffix .o, $(basename $(UI_TEST_SOURCES)))

LIBS =
CXXFLAGS = -Wall -Wformat
# CXXFLAGS += -g
CXXFLAGS += -IUI_LIB
CXXFLAGS += -IUI_LIB/libs/gl3w
CXXFLAGS += -IUI_LIB/libs/glfw/include
CFLAGS = $(CXXFLAGS)

ifeq ($(OS),Windows_NT) #WINDOWS
   ECHO_MESSAGE = Windows
   MAKE = mingw32-make
   RM = del
   UI_FLAG = -mwindows

   SERVER_EXE = $(BINDIR)$(addsuffix .exe, $(basename $(SERVER)))
   CLIENT_EXE = $(BINDIR)$(addsuffix .exe, $(basename $(CLIENT)))
   UI_TEST_EXE = $(BINDIR)$(addsuffix .exe, $(basename $(UI_TEST)))

   LIBS = -lgdi32 -lopengl32 -limm32 -lws2_32 -lstdc++
   DLLS += UI_LIB/libs/glfw/lib-mingw-w64/libglfw3.a
   DLLS += UI_LIB/libs/glfw/lib-mingw-w64/libglfw3dll.a
   CXXFLAGS += -static
   CFLAGS = $(CXXFLAGS)
else
	UNAME_S := $(shell uname -s)
    ifeq ($(UNAME_S),Linux) # LINUX
		ECHO_MESSAGE = Linux

		LIBS = -lGL `pkg-config --static --libs glfw3`
		CXXFLAGS += `pkg-config --cflags glfw3`
		CFLAGS = $(CXXFLAGS)
	endif
endif

all: server client
	@-$(RM) $(SERVER_OBJS)
	@-$(RM) $(CLIENT_OBJS)
	@-$(RM) $(UTILS_OBJS)
	@echo [Build complete for $(ECHO_MESSAGE)]

%.o:%.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

imgui: clean_imgui_objs
	@echo [Building imgui]
	cd UI_LIB && $(MAKE)

server: $(SERVER_OBJS) $(UTILS_OBJS)
	$(CXX) -o $(SERVER_EXE) $(SERVER_OBJS) $(UTILS_OBJS) $(CXXFLAGS)

client: imgui $(CLIENT_OBJS) $(UTILS_OBJS)
	$(CXX) -o $(CLIENT_EXE) $(CLIENT_OBJS) $(UTILS_OBJS) $(IMGUI_OBJS) $(DLLS) $(CXXFLAGS) $(UI_FLAG) $(LIBS)

UI_Test: imgui $(UI_TEST_OBJS)
	$(CXX) -o $(UI_TEST_EXE) $(UI_TEST_OBJS) $(IMGUI_OBJS) $(DLLS) $(CXXFLAGS) $(LIBS)
	-$(RM) $(UI_TEST_OBJS)
	@echo Build ui_test complete for $(ECHO_MESSAGE)

clean: clean_imgui_objs
	 -$(RM) $(CLIENT_OBJS)
	 -$(RM) $(SERVER_OBJS)
	 -$(RM) $(SERVER_EXE)
	 -$(RM) $(CLIENT_EXE)
	 -$(RM) $(UI_TEST_EXE)

clean_imgui_objs:
	 -cd UI_LIB && $(RM) $(IMGUI_NOTDIR_OBJS)

.PHONY : clean
.PHONY : clean_imgui_objs
