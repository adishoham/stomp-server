# Compiler and Flags
CXX := g++
CFLAGS := -c -Wall -Weffc++ -g -std=c++11 -Iinclude
LDFLAGS := -lboost_system -lpthread

# Directories
SRC_DIR := src
BIN_DIR := bin

# Targets
all: $(BIN_DIR)/EchoClient $(BIN_DIR)/StompWCIClient $(BIN_DIR)/client export-path run-client

# Ensure bin directory exists before compilation
$(BIN_DIR):
	mkdir -p $(BIN_DIR)

# Alias for StompWCIClient so make StompWCIClient works
StompWCIClient: $(BIN_DIR)/StompWCIClient

# Alias for EchoClient so make EchoClient works
EchoClient: $(BIN_DIR)/EchoClient

# Echo Client Build
$(BIN_DIR)/EchoClient: $(BIN_DIR)/ConnectionHandler.o $(BIN_DIR)/echoClient.o $(BIN_DIR)/event.o | $(BIN_DIR)
	$(CXX) -o $@ $^ $(LDFLAGS)

# Stomp Client Build
$(BIN_DIR)/StompWCIClient: $(BIN_DIR)/ConnectionHandler.o $(BIN_DIR)/StompClient.o $(BIN_DIR)/event.o $(BIN_DIR)/Client.o | $(BIN_DIR)
	$(CXX) -o $@ $^ $(LDFLAGS)

# Create a symlink for StompWCIClient as bin/client
$(BIN_DIR)/client: $(BIN_DIR)/StompWCIClient
	ln -sf StompWCIClient $@

# Automatically add bin/ to PATH
export-path:
	@echo 'export PATH=$(PWD)/bin:$$PATH' >> ~/.bashrc
	@echo 'export PATH=$(PWD)/bin:$$PATH' >> ~/.zshrc
	@if [ -n "$$BASH_VERSION" ]; then . ~/.bashrc; fi
	@if [ -n "$$ZSH_VERSION" ]; then . ~/.zshrc; fi

# Run the client automatically after build
run-client: $(BIN_DIR)/client
	@./bin/client

# Compilation Rules
$(BIN_DIR)/ConnectionHandler.o: $(SRC_DIR)/ConnectionHandler.cpp include/ConnectionHandler.h | $(BIN_DIR)
	$(CXX) $(CFLAGS) -o $@ $<

$(BIN_DIR)/echoClient.o: $(SRC_DIR)/echoClient.cpp | $(BIN_DIR)
	$(CXX) $(CFLAGS) -o $@ $<

$(BIN_DIR)/event.o: $(SRC_DIR)/event.cpp include/event.h | $(BIN_DIR)
	$(CXX) $(CFLAGS) -o $@ $<

$(BIN_DIR)/StompClient.o: $(SRC_DIR)/StompClient.cpp | $(BIN_DIR)
	$(CXX) $(CFLAGS) -o $@ $<

$(BIN_DIR)/Client.o: $(SRC_DIR)/Client.cpp include/Client.h | $(BIN_DIR)
	$(CXX) $(CFLAGS) -o $@ $<

# Clean Build
.PHONY: clean
clean:
	rm -rf $(BIN_DIR)/*
