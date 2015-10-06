#include <ctype.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <sys/msg.h>

#include "message.h"

int main(void) {
    int running = 1;
    int msgid;
    message_t client_request;
    char *tmp_char_ptr;
    long int receive_type = 1;

    // Setup the message queue
    msgid = msgget((key_t) 1234, 0666 | IPC_CREAT);

    if (msgid == -1) {
    	fprintf(stderr, "msgget failed with error: %d\n", errno);
    	exit(EXIT_FAILURE);
    }

    while(running) {
        if (msgrcv(msgid, (void *)&client_request, sizeof(client_request.packet), receive_type, 0) == -1) {
    	    fprintf(stderr, "msgrcv failed with error: %d\n", errno);
    	    running = 0;
	    }
    	client_request.msg_type = (long int) client_request.packet.pid;
    	tmp_char_ptr = client_request.packet.name;
    	while(*tmp_char_ptr) {
    	    *tmp_char_ptr = toupper(*tmp_char_ptr);
    	    tmp_char_ptr++;
    	}
    	if (msgsnd(msgid, (void *)&client_request, BUFFER_SIZE, 0) == -1) {
    	    fprintf(stderr, "msgsnd failed: %d\n", errno);
    	    exit(EXIT_FAILURE);
    	}
    }

    exit(EXIT_SUCCESS);
}
