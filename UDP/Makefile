CC := gcc

chat:
	$(CC) -o chat chat_ui.c list.c gap_buffer.c -lncurses

test_gap_buffer:
	$(CC) -o test_gap_buffer gap_buffer.c test_gap_buffer.c && ./test_gap_buffer

server:
	$(CC) -o server server.c

client:
	$(CC) -o client client.c

all: server client

clean:
	rm -f *.o server client chat test_gap_buffer
