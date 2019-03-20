user2: user2.c rsocket.h librsocket.a user1
	gcc user2.c -g -pthread -L. -lrsocket -o user2

user1: user1.c rsocket.h librsocket.a
	gcc user1.c -pthread -L. -lrsocket -o user1

librsocket.a: rsocket.o
	ar -rcs librsocket.a rsocket.o

rsocket.o: rsocket.c rsocket.h
	gcc -Wall -c rsocket.c

clean:
	rm rsocket.o librsocket.a user1 user2