DIR_BIN = ./bin
DIR_OBJ = ./obj
DIR_SRC = ./src

SRC = $(wildcard $(DIR_SRC)/*.cpp)
OBJ = $(patsubst %.cpp, $(DIR_OBJ)/%.o, $(notdir $(SRC)))

TARGET = main
BIN_TARGET = $(DIR_BIN)/$(TARGET)

CC = g++
INCLUDE = /home/hy/Work/cpp/3rd/boost_1_67_0
CFLAGS = -g -Wall


all: $(BIN_TARGET)

$(BIN_TARGET): $(OBJ)
	$(CC) $< -o $@

$(OBJ): $(SRC)
	$(CC) $(CFLAGS) -c -I$(INCLUDE) $< -o $@

clean:
	rm -rf $(DIR_BIN)/helloworld
	rm -rf $(DIR_OBJ)/*.o