CC := gcc

chat:
	$(CC) -o chat chat_ui.c list.c -lncurses

server:
	$(CC) -o server server.c

client:
	$(CC) -o client client.c

all: server client

clean:
	rm -f *.o server client chat
