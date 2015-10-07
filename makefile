CC=gcc
SERV=controller.c message.c
DEVICE=device.c message.c
CLOUD=cloud.c message.c
TEST=test.c message.c

all:
	gcc -o controller $(SERV)
	gcc -o device $(DEVICE)
	gcc -o cloud $(CLOUD)
	gcc -o test $(TEST)

device.o: message.h
server.o: message.h
cloud.o: message.h
test.o: message.h

clean:
	rm -f client *.o device message controller test cloud msgQ.tar.gz

package:
	tar -zcvf msgQ.tar.gz controller.c device.c cloud.c message.h message.c makefile
