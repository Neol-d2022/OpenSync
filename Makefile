CC=gcc

CFLAGS=-Wall -Wextra -Ofast
LFLAGS=

OBJS=main.o
DEPS=
LIBS=-lpthread

BIN=OpenSync

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS) -DDEBUG -g3

$(BIN): $(OBJS)
	$(CC) -o $@ $^ $(LFLAGS) $(LIBS)

clean:
	rm -f $(OBJS) $(BIN)