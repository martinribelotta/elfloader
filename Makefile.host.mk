SRC=main.c
OBJ=$(SRC:.c=.o)

TARGET=elfloader

CC=gcc
LD=gcc

CFLAGS=-O0 -ggdb3
LDFLAGS=

all: $(TARGET)

$(TARGET): $(OBJ)
	$(LD) -o $(TARGET) $(LDFLAGS) $(OBJ)

clean:
	rm -fR *.o $(TARGET)

debug:
	gdb $(TARGET)
