
.PHONY: all
all: hash_client hash_server

hash_client: hash_client.c
	gcc -std=c99 -o hash_client hash_client.c -Wall -pedantic -lpthread

hash_server: hash_server.c
	gcc -std=c99 hash_server.c crc32.c -o hash_server -Wall -pedantic-errors -lpthread

clean:
	rm -f hash_server hash_client hash_server.o hash_client.o crc32.o logfile.txt

