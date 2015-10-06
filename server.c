#include <ctype.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <signal.h>
#include <sys/msg.h>

#include "message.h"

#define RECEIVE_TYPE -2L

int main(void) {
    int running = 1;
    int msgid;
    message_t data_received, data_transmition;

    // Setup the message queue
    msgid = msgget((key_t) 1234, 0666 | IPC_CREAT);

    if (msgid == -1) {
    	fprintf(stderr, "msgget failed with error: %d\n", errno);
    	exit(EXIT_FAILURE);
    }

    while(1) {
        receive_msg(msgid, (void *)&data_received, sizeof(data_received.packet), RECEIVE_TYPE, 0);
        
    	data_transmition.msg_type = (long int) data_received.packet.pid;
        data_transmition.packet.pid = getpid();
        data_transmition.packet.value = 1;
    	
    	send_msg(msgid, (void *)&data_transmition, sizeof(data_transmition.packet), 0);
    }

    exit(EXIT_SUCCESS);
}
