#include <ctype.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <sys/msg.h>

#include "message.h"

#define RECEIVE_TYPE -2L
#define MAX_DEVICES 8

typedef struct {
    pid_t pid;
    int type;
    int threshold;
    char name[BUFFER_SIZE];
} device_t;

// Register a device with the controller by adding it to the devices array.
void register_device(device_t *device, message_t *msg) {
    device->pid = msg->packet.pid;
    device->type = msg->packet.type;
    device->threshold = msg->packet.threshold;
    strcpy(device->name, msg->packet.name);
}

// Returns the index of the device that matches the sender of the message.
// Returns -1 if it's not found.
int find_device(message_t *msg, device_t devices[MAX_DEVICES]) {
    for (int i = 0; i < MAX_DEVICES; i++) {
        if (devices[i].pid == msg->packet.pid) {
            return i;
        }
    }
    return -1;
}


int main(void) {
    int msgid;
    message_t data_received, data_transmition;
    device_t current_device, devices[MAX_DEVICES];
    int next_device = 0;
    int current_index;

    // Setup the message queue
    msgid = msgget((key_t) 1234, 0666 | IPC_CREAT);

    if (msgid == -1) {
    	fprintf(stderr, "msgget failed with error: %d\n", errno);
    	exit(EXIT_FAILURE);
    }

    while(1) {
        receive_msg(msgid, (void *)&data_received, sizeof(data_received.packet), RECEIVE_TYPE, 0);
        

        if (data_received.msg_type == REGISTER_KEY) {
            register_device(&devices[next_device++], &data_received);
        } else if (data_received.msg_type == MESSAGE_KEY) {
            current_index = find_device(&data_received, &devices, MAX_DEVICES);
            if (find_device(&devices, &data_received) == -1) {
                continue;
            }
            current = devices[current_index];
        }
        
    	data_transmition.msg_type = (long int) data_received.packet.pid;
        data_transmition.packet.pid = getpid();
        data_transmition.packet.value = 1;
    	
    	send_msg(msgid, (void *)&data_transmition, sizeof(data_transmition.packet), 0);
    }

    exit(EXIT_SUCCESS);
}
