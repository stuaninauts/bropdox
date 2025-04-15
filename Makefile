# -------------------------------
# Dropbox-like Project (Modular)
# -------------------------------
# Directory Structure:
# .
# ├── client/
# │   ├── include/
# │   └── src/
# ├── server/
# │   ├── include/
# │   └── src/
# ├── shared/
# │   ├── include/
# │   └── src/
# ├── build/
# └── bin/

# Compiler and Flags
CXX := g++
CXXFLAGS := -std=c++17 -Wall -Wextra
LDFLAGS := -pthread

# Include paths
INCLUDES := -I./client/include \
            -I./server/include \
            -I./shared/include

# Directories
BUILD_DIR := build
BIN_DIR := bin

# Targets
CLIENT_TARGET := $(BIN_DIR)/client
SERVER_TARGET := $(BIN_DIR)/server

# Source files
CLIENT_SRCS := $(wildcard client/src/*.cpp)
SERVER_SRCS := $(wildcard server/src/*.cpp)
SHARED_SRCS := $(wildcard shared/src/*.cpp)

# Object files
CLIENT_OBJS := $(patsubst client/src/%.cpp,$(BUILD_DIR)/client/%.o,$(CLIENT_SRCS))
SERVER_OBJS := $(patsubst server/src/%.cpp,$(BUILD_DIR)/server/%.o,$(SERVER_SRCS))
SHARED_OBJS := $(patsubst shared/src/%.cpp,$(BUILD_DIR)/shared/%.o,$(SHARED_SRCS))

# Rules
all: dirs $(CLIENT_TARGET) $(SERVER_TARGET)

dirs:
	@mkdir -p $(BUILD_DIR)/client
	@mkdir -p $(BUILD_DIR)/server
	@mkdir -p $(BUILD_DIR)/shared
	@mkdir -p $(BIN_DIR)

$(CLIENT_TARGET): $(CLIENT_OBJS) $(SHARED_OBJS)
	$(CXX) $^ -o $@ $(LDFLAGS)

$(SERVER_TARGET): $(SERVER_OBJS) $(SHARED_OBJS)
	$(CXX) $^ -o $@ $(LDFLAGS)

$(BUILD_DIR)/client/%.o: client/src/%.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

$(BUILD_DIR)/server/%.o: server/src/%.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

$(BUILD_DIR)/shared/%.o: shared/src/%.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

clean:
	rm -rf $(BUILD_DIR) $(BIN_DIR)

.PHONY: all dirs clean