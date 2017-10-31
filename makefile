all: deliver server

deliver: deliver.o
	gcc deliver.c -o deliver

server: server.c
	gcc server.c -o server
