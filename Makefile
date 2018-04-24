CXX = g++
CC = gcc
MAKE = make
BINPATH = bin/
SRCPATH = src/
LIBPATH = UI_LIB/
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
UI_TEST_SOURCES = UI_Test.cpp
IMGUI_SOURCES = $(LIBPATH)imgui_impl_glfw_gl3.cpp $(LIBPATH)imgui.cpp $(LIBPATH)imgui_demo.cpp $(LIBPATH)imgui_draw.cpp

OBJS = $(addsuffix .o, $(basename $(SOURCES)))
SERVER_OBJS = $(addsuffix .o, $(basename $(SERVER_SOURCES)))
CLIENT_OBJS = $(addsuffix .o, $(basename $(CLIENT_SOURCES)))
UTILS_OBJS = $(addsuffix .o, $(basename $(UTILS_SOURCES)))
IMGUI_OBJS = $(addsuffix .o, $(basename $(IMGUI_SOURCES)))
IMGUI_OBJS += $(LIBPATH)gl3w.o
IMGUI_NODIR_OBJS = $(notdir $(IMGUI_OBJS))
UI_TEST_OBJS = $(addsuffix .o, $(basename $(UI_TEST_SOURCES)))

LIBS =
CXXFLAGS = -Wall -Wformat
# CXXFLAGS += -g
CXXFLAGS += -I$(LIBPATH)
CXXFLAGS += -I$(LIBPATH)libs/gl3w
CXXFLAGS += -I$(LIBPATH)libs/glfw/include
CFLAGS = $(CXXFLAGS)

ifeq ($(OS),Windows_NT) #WINDOWS
   ECHO_MESSAGE = Windows
   MAKE = mingw32-make
   RM = del
   UI_FLAG = -mwindows

   SERVER_EXE = $(addsuffix .exe, $(SERVER))
   CLIENT_EXE = $(addsuffix .exe, $(CLIENT))
   UI_TEST_EXE = $(addsuffix .exe, $(UI_TEST))

   LIBS = -lgdi32 -lopengl32 -limm32 -lws2_32 -lstdc++
   DLLS += $(LIBPATH)libs/glfw/lib-mingw-w64/libglfw3.a
   DLLS += $(LIBPATH)libs/glfw/lib-mingw-w64/libglfw3dll.a
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

%.o:$(SRCPATH)%.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

imgui: clean_imgui_objs
	@echo [Building imgui]
	cd $(LIBPATH) && $(MAKE)

server: $(SERVER_OBJS) $(UTILS_OBJS)
	$(CXX) -o $(BINPATH)$(SERVER_EXE) $(SERVER_OBJS) $(UTILS_OBJS) $(CXXFLAGS) $(LIBS)

client: imgui $(CLIENT_OBJS) $(UTILS_OBJS)
	$(CXX) -o $(BINPATH)$(CLIENT_EXE) $(CLIENT_OBJS) $(UTILS_OBJS) $(IMGUI_OBJS) $(DLLS) $(CXXFLAGS) $(UI_FLAG) $(LIBS)

ui_test: imgui $(UI_TEST_OBJS)
	$(CXX) -o $(BINPATH)$(UI_TEST_EXE) $(UI_TEST_OBJS) $(IMGUI_OBJS) $(DLLS) $(CXXFLAGS) $(LIBS)
	-$(RM) $(UI_TEST_OBJS)
	@echo Build UI_Test complete for $(ECHO_MESSAGE)

clean: clean_imgui_objs
	 -$(RM) $(CLIENT_OBJS)
	 -$(RM) $(SERVER_OBJS)
	 -$(RM) $(UTILS_OBJS)
	 -cd $(BINPATH) && $(RM) $(SERVER_EXE)
	 -cd $(BINPATH) && $(RM) $(CLIENT_EXE)
	 -cd $(BINPATH) && $(RM) $(UI_TEST_EXE)

clean_imgui_objs:
	 -cd $(LIBPATH) && $(RM) $(IMGUI_NODIR_OBJS)

.PHONY : clean
.PHONY : clean_imgui_objs
