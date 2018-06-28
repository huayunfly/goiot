DIR_BIN = ./bin
DIR_OBJ = ./obj
DIR_SRC = ./src
DIR_TEST = ./test

#SRC = $(wildcard $(DIR_SRC)/*.cpp)
#OBJ = $(patsubst %.cpp, $(DIR_OBJ)/%.o, $(notdir $(SRC)))
OBJS = $(DIR_OBJ)/main.o $(DIR_OBJ)/hashtable.o $(DIR_OBJ)/buffer.o 

TARGET = main
BIN_TARGET = $(DIR_BIN)/$(TARGET)

CC = g++
INCLUDE = -I/home/hy/Work/cpp/3rd/boost_1_67_0
CFLAGS = -g -Wall -std=c++17


all: $(BIN_TARGET)

debug: $(CFLAGS) += -DDEBUG
debug: $(BIN_TARGET)

$(BIN_TARGET): $(OBJS)
	$(CC) $(OBJS) -o $@

$(DIR_OBJ)/%.o: $(DIR_SRC)/%.cpp
	$(CC) $(CFLAGS) $(INCLUDE) -c $< -o $@

clean:
	rm -rf $(DIR_BIN)/*
	rm -rf $(DIR_OBJ)/*.o

test:
	(cd $(DIR_TEST);$(MAKE))