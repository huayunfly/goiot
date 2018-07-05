DIR_BIN = ../bin
DIR_OBJ = ../obj
DIR_SRC = .
DIR_SRC_EXT = ../src

OBJS = $(DIR_OBJ)/test_tags.o $(DIR_OBJ)/hashtable.o
OBJS2 = $(DIR_OBJ)/test_buffer.o
OBJS3 = $(DIR_OBJ)/test_socket.o $(DIR_OBJ)/socketservice.o


BIN_TARGET = $(DIR_BIN)/test_tags
BIN_TARGET2 = $(DIR_BIN)/test_buffer
BIN_TARGET3 = $(DIR_BIN)/test_socket

CC = g++
INCLUDE = -I/home/hy/Work/cpp/3rd/boost_1_67_0
CFLAGS = -g -Wall -std=c++17
LIBS = -lpthread

all: $(BIN_TARGET) $(BIN_TARGET2) $(BIN_TARGET3)

# TARGET
$(BIN_TARGET): $(OBJS) 
	$(CC) $(OBJS) -o $@ 

$(DIR_OBJ)/%.o: $(DIR_SRC)/%.cpp
	$(CC) $(CFLAGS) $(INCLUDE) -c $< -o $@

$(DIR_OBJ)/%.o: $(DIR_SRC_EXT)/%.cpp
	$(CC) $(CFLAGS) $(INCLUDE) -c $< -o $@

# TARGET2
$(BIN_TARGET2): $(OBJS2) 
	$(CC) $(OBJS2) $(LIBS) -o $@ 

$(DIR_OBJ)/%.o: $(DIR_SRC)/%.cpp
	$(CC) $(CFLAGS) $(INCLUDE) -c $< -o $@

$(DIR_OBJ)/%.o: $(DIR_SRC_EXT)/%.cpp
	$(CC) $(CFLAGS) $(INCLUDE) -c $< -o $@

# TARGET3
$(BIN_TARGET3): $(OBJS3) 
	$(CC) $(OBJS3) $(LIBS) -o $@ 

$(DIR_OBJ)/%.o: $(DIR_SRC)/%.cpp
	$(CC) $(CFLAGS) $(INCLUDE) -c $< -o $@

$(DIR_OBJ)/%.o: $(DIR_SRC_EXT)/%.cpp
	$(CC) $(CFLAGS) $(INCLUDE) -c $< -o $@

clean:
	rm -rf $(DIR_BIN)/test_*
	rm -rf $(DIR_OBJ)/test_*.o
	rm -rf $(DIR_OBJ)/*.o