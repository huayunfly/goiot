DIR_BIN = ../bin
DIR_OBJ = ../obj
DIR_SRC = .
DIR_SRC_EXT = ../src

OBJS = $(DIR_OBJ)/test_tags.o $(DIR_OBJ)/hashtable.o

BIN_TARGET = $(DIR_BIN)/test_tags

CC = g++
INCLUDE = -I/home/hy/Work/cpp/3rd/boost_1_67_0
CFLAGS = -g -Wall -std=c++17

all: $(BIN_TARGET)

$(BIN_TARGET): $(OBJS) 
	$(CC) $(OBJS) -o $@ 

$(DIR_OBJ)/%.o: $(DIR_SRC)/%.cpp
	$(CC) $(CFLAGS) $(INCLUDE) -c $< -o $@

$(DIR_OBJ)/%.o: $(DIR_SRC_EXT)/%.cpp
	$(CC) $(CFLAGS) $(INCLUDE) -c $< -o $@

clean:
	rm -rf $(DIR_BIN)/test_*
	rm -rf $(DIR_OBJ)/test_*.o
	rm -rf $(DIR_OBJ)/*.o