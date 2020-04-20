EXEC = memory

CFLAGS  = -W -Wall -O2
LDFLAGS = 
CC = cc

SRC=$(wildcard *.c)
OBJ=$(SRC:.c=.o)

all: $(EXEC)

static: CFLAGS += -march=x86-64
static: LDFLAGS += -static
static: CC = x86_64-pc-linux-musl-gcc
static: clean $(EXEC)

$(EXEC): $(OBJ)
	$(CC) -o $@ $^ $(LDFLAGS)

%.o: %.c
	$(CC) -o $@ -c $< $(CFLAGS)

clean:
	rm -fv *.o

mrproper: clean
	rm -fv $(EXEC)

