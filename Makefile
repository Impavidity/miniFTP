.PHONY:clean
CC=g++
CFLAGS=-Wall -g
BIN=miniftpd
OBJS=main.o util.o
$(BIN):$(OBJS)
	$(CC) $(CFLAGS) $^ -o $@
%.o:%.c
	$(CC) $(CFLAGS) -c $< -o $@
clean:
	rm -f *.o $(BIN)