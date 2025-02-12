# Compiler and compiler flags
CXX = g++
CXXFLAGS = -Wall -Ilib/imgui -lglfw3 -lkernel32 -lopengl32 -lglu32 -lglew32 -lwinmm

# Source files directory and wildcard for all .cpp files
SRC_DIR = .
IMGUI_DIR = lib/imgui
IMGUI_BACKENDS_DIR = $(IMGUI_DIR)/backends
SRCS = $(wildcard $(SRC_DIR)/*.cpp) \
       $(IMGUI_DIR)/imgui.cpp \
       $(IMGUI_DIR)/imgui_draw.cpp \
       $(IMGUI_DIR)/imgui_tables.cpp \
       $(IMGUI_DIR)/imgui_widgets.cpp \
       $(IMGUI_BACKENDS_DIR)/imgui_impl_glfw.cpp \
       $(IMGUI_BACKENDS_DIR)/imgui_impl_opengl3.cpp

# Object files directory and naming
OBJ_DIR = obj
OBJS = $(patsubst $(SRC_DIR)/%.cpp,$(OBJ_DIR)/%.o,$(SRCS))

# Executable name
EXEC = run.exe

# Default target
all: $(EXEC) run 

# Linking object files into the executable
$(EXEC): $(OBJS)
	$(CXX) -o $@ $^ $(CXXFLAGS)

# Compiling each source file into object files
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp | $(OBJ_DIR)
	$(CXX) -c -o $@ $< $(CXXFLAGS)

# Create object directory if it doesn't exist
$(OBJ_DIR):
	mkdir $(OBJ_DIR)



# Clean rule to remove compiled files and executable
clean:
	rm -f $(OBJ_DIR)\* 
	rm -f $(EXEC)

# Phony target to run the executable
run: $(EXEC)
	.\$(EXEC)

# Phony targets
.PHONY: clean all run
