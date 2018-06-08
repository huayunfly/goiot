DIR_BIN = ./bin
DIR_OBJ = ./obj
DIR_SRC = ./src
DIR_TEST = ./test

SRC = $(wildcard $(DIR_SRC)/*.cpp)
OBJ = $(patsubst %.cpp, $(DIR_OBJ)/%.o, $(notdir $(SRC)))

TARGET = main
BIN_TARGET = $(DIR_BIN)/$(TARGET)

CC = g++
INCLUDE = -I/home/hy/Work/cpp/3rd/boost_1_67_0
CFLAGS = -g -Wall -std=c++17


all: $(BIN_TARGET)

$(BIN_TARGET): $(DIR_OBJ)/main.o
	$(CC) $< -o $@

$(DIR_OBJ)/%.o: $(DIR_SRC)/%.cpp
	$(CC) $(CFLAGS) -c $(INCLUDE) $< -o $@

clean:
	rm -rf $(DIR_BIN)/*.*
	rm -rf $(DIR_OBJ)/*.o

subsystem:
	cd $(DIR_TEST) && make