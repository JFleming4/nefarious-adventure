CC=gcc
SERV=controller.c message.c
DEVICE=device.c message.c

all:
	gcc -o controller $(SERV)
	gcc -o device $(DEVICE)

device.o: message.h
server.o: message.h
# message.o: message.h

clean:
	rm -f client *.o device message controller test msgQ.tar.gz

package:
	tar -zcvf msgQ.tar.gz controller.c device.c message.h message.c makefile
