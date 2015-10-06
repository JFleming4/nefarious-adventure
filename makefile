CC=gcc
SERV=server.c message.c
DEVICE=device.c message.c

all:
	gcc -o server $(SERV)
	gcc -o device $(DEVICE)

device.o: message.h
server.o: message.h
# message.o: message.h

clean:
	rm -f client *.o device message server test msgQ.tar.gz

package:
	tar -zcvf msgQ.tar.gz server.c device.c message.h message.c makefile
