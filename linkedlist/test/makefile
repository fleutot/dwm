CC = gcc
CFLAGS = -std=c99 -g -Wall -O3 -Wno-unused-function

SRC = ../linkedlist.c linkedlist_test.c
OBJ = $(SRC:.c=.o)
TARGET = linkedlist_test

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) $(OBJ) -o $(TARGET)

.c.o:
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	$(RM) ../*.o *.o $(TARGET)

test: $(TARGET)
	./$(TARGET)
