CXX = g++
CC = gcc
MAKE = make
EXE = Message
UI_TEST_EXE = ui_test
RM = rm

SOURCES =
IMGUI_SOURCES = UI_LIB/imgui_impl_glfw_gl3.cpp UI_LIB/imgui.cpp UI_LIB/imgui_demo.cpp UI_LIB/imgui_draw.cpp
UI_TEST_SOURCES = ui_test.cpp

OBJS = $(addsuffix .o, $(basename $(SOURCES)))
IMGUI_OBJS = $(addsuffix .o, $(basename $(IMGUI_SOURCES)))
IMGUI_OBJS += UI_LIB/gl3w.o
UI_TEST_OBJS = $(addsuffix .o, $(basename $(UI_TEST_SOURCES)))

LIBS =
CXXFLAGS = -Wall -Wformat
CXXFLAGS += -IUI_LIB
CXXFLAGS += -IUI_LIB/libs/gl3w
CXXFLAGS += -IUI_LIB/libs/glfw/include
CFLAGS = $(CXXFLAGS)

ifeq ($(OS),Windows_NT) #WINDOWS
   ECHO_MESSAGE = "Windows"
   MAKE = mingw32-make
   RM = del

   LIBS = -lgdi32 -lopengl32 -limm32
   DLLS += UI_LIB/libs/glfw/lib-mingw-w64/libglfw3.a
   DLLS += UI_LIB/libs/glfw/lib-mingw-w64/libglfw3dll.a
else
	UNAME_S := $(shell uname -s)
    ifeq ($(UNAME_S),Linux)
		ECHO_MESSAGE = "Linux"
		LIBS = -lGL `pkg-config --static --libs glfw3`
		CXXFLAGS += `pkg-config --cflags glfw3`
		CFLAGS = $(CXXFLAGS)
	endif
endif

all:$(EXE)
	@echo Build complete for $(ECHO_MESSAGE)

%.o:%.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

imgui:
	@echo making imgui
	cd UI_LIB && $(MAKE)

$(EXE): imgui $(OBJS)
	$(CXX) -o $(EXE) $(OBJS) $(IMGUI_OBJS) $(DLLS) $(CXXFLAGS) $(LIBS)
	$(RM) $(OBJS)

ui_test: imgui $(UI_TEST_OBJS)
	$(CXX) -o $(UI_TEST_EXE) $(UI_TEST_OBJS) $(IMGUI_OBJS) $(DLLS) $(CXXFLAGS) $(LIBS)
	$(RM) $(UI_TEST_OBJS)

clean:
	$(RM) $(EXE) && $(RM) $(UI_TEST_EXE)