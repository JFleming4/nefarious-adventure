CC=gcc
SERV=controller.c message.c
DEVICE=device.c message.c
CLOUD=cloud.c message.c

all:
	gcc -o controller $(SERV)
	gcc -o device $(DEVICE)
	gcc -o cloud $(CLOUD)
	gcc -o main main.c

device.o: message.h
server.o: message.h
cloud.o: message.h

clean:
	rm -f client *.o device message controller cloud main msgQ.tar.gz

package:
	tar -zcvf msgQ.tar.gz controller.c device.c cloud.c main.c message.h message.c makefile
