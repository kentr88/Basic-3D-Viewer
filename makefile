# Compiler and compiler flags
CXX = g++
CXXFLAGS = -Wall -lglfw3 -lkernel32 -lopengl32 -lglu32 -lglew32 -lwinmm

# Source files directory and wildcard for all .cpp files
SRC_DIR = .
SRCS = $(wildcard $(SRC_DIR)/*.cpp)

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
	del /Q $(OBJ_DIR)\* 2>nul
	del /Q $(EXEC) 2>nul

# Phony target to run the executable
run: $(EXEC)
	.\$(EXEC)

# Phony targets
.PHONY: clean all run
