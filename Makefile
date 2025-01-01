# Define the compiler
CXX = g++

# Define include paths
INCLUDES = -I./src -I./src/handler -I./src/database

# Define the targets
TARGETS = server

# Define the source files for each target
SERVER_SRC = src/server.cpp src/database/*.cpp src/handler/*.cpp src/util.cpp

# Define the libraries for the server
SERVER_LIBS = -lmysqlcppconn

# Default target
all: $(TARGETS)

# Rule to build the server
server: $(SERVER_SRC)
	$(CXX) $(INCLUDES) -o $@ $^ $(SERVER_LIBS)

# Clean up build files
clean:
	rm -f $(TARGETS)