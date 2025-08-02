CC := gcc

test_gap_buffer:
	$(CC) -o test_gap_buffer gap_buffer.c ./tests/test_gap_buffer.c && ./test_gap_buffer

server:
	make clean && $(CC) -o server server.c

client:
	$(CC) -o client client.c list.c gap_buffer.c -lncurses -pthread

all: server client

clean:
	rm -f *.o server client chat test_gap_buffer
