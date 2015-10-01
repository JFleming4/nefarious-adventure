all: client server

client.o: message.h
server.o: message.h

clean:
	rm -f client *.o server msgQ.tar.gz

package:
	tar -zcvf msgQ.tar.gz server.c client.c message.h makefile

