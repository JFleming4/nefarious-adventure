CC=gcc
SERV=controller.c message.c
DEVICE=device.c message.c
CLOUD=cloud.c message.c

all:
	gcc -o controller $(SERV)
	gcc -o device $(DEVICE)
	gcc -o cloud $(CLOUD)

device.o: message.h
server.o: message.h
cloud.o: message.h

clean:
	rm -f client *.o device message controller cloud msgQ.tar.gz

package:
	tar -zcvf msgQ.tar.gz controller.c device.c cloud.c message.h message.c makefile
