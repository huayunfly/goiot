DIR_BIN = ../bin
DIR_OBJ = ../obj/test
DIR_SRC = .

SRC = $(wildcard $(DIR_SRC)/*.cpp)
OBJ = $(patsubst %.cpp, $(DIR_OBJ)/%.o, $(notdir $(SRC)))

BIN_TARGET = $(patsubst %.cpp, $(DIR_BIN)/%, $(notdir $(SRC)))

CC = g++
INCLUDE = -I/home/hy/Work/cpp/3rd/boost_1_67_0
CFLAGS = -g -Wall -std=c++17


all: $(BIN_TARGET)

$(BIN_TARGET): $(OBJ)
	$(CC) $< -o $@

$(OBJ): $(SRC)
	$(CC) $(CFLAGS) -c $(INCLUDE) $< -o $@

clean:
	rm -rf $(DIR_BIN)/test_*.*
	rm -rf $(DIR_OBJ)/*.o