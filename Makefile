all:
	gcc -c communication.c -o communication.o
	gcc -c server.c -o server.o
	gcc -c client.c -o client.o
	gcc communication.o server.o -o server
	gcc communication.o client.o -o client

clean:
	rm -f server client
	rm -f server.o client.o
	rm -f communication.o
