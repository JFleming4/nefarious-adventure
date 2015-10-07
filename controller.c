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

struct device {
    pid_t pid;
    int type;
    int threshold;
    char name[BUFFER_SIZE];
    char has_partner;
    struct device *partner;
};

typedef struct device device_t;
void print_device(device_t *device) {
    const char *type = (device->type) ? "ACTUATOR" : "SENSOR";
    const char *p = (device->has_partner) ? "TRUE" : "FALSE";
    printf("Name: %s\nPid: %d\nThreshold: %d\nDevice Type: %s\nHas Partner: %s\n",
           device->name,
           device->pid,
           device->threshold,
           type,
           p
    );
}
// Register a device with the controller by adding it to the devices array.
void register_device(message_t *msg, device_t devices[MAX_DEVICES], int next_device) {
    device_t *device = &devices[next_device];

    device->pid = (long int) msg->packet.pid;
    device->type = msg->packet.type;
    device->threshold = msg->packet.threshold;
    strcpy(device->name, msg->packet.name);
    printf("register device after strcpy\n");
    printf("msg name: %s\n device name: %s\n", msg->packet.name, device->name);
    // If there are already devices being tracked, loop through all of them.
    // If there is a device of the opposite type without a partner then make
    // them each others partner.
    if (next_device) {
        printf("next device: %d\n", next_device);
        for (int i = 0; i < next_device; i++) {
            if ((devices[i].type != msg->packet.type) && !devices[i].has_partner) {
                devices[i].has_partner = 1;
                devices[i].partner = device;
                device->has_partner = 1;
                device->partner = &devices[i];
                
                printf("1 Device: %d, is partnered with: %d\n", device->pid, device->partner -> pid);
                print_device(device);
                
                printf("2 Device: %d, is partnered with: %d\n", devices[i].pid, devices[i].partner -> pid);
                print_device(&devices[i]);
                return;
            }
        }
    }

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

void cpy_device_msg(message_t *msg, device_t *device) {
    strcpy(msg->packet.name, device->name);
    msg->packet.threshold = device->threshold;
    msg->packet.type = device->type;
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

    for (int i = 0; i < MAX_DEVICES; i++) {
        devices[i].has_partner = 0;
    }

    // The Transmitted data will always be from here.
    data_transmition.packet.pid = getpid();

    while(1) {
        receive_msg(msgid, (void *)&data_received, sizeof(data_received.packet), RECEIVE_TYPE, 0);

        // Registration vs Message Section
        if (data_received.msg_type == REGISTER_KEY) {
            register_device(&data_received, devices, next_device);
            cpy_device_msg(&data_transmition, &devices[next_device]);
            data_transmition.msg_type = (long int) data_received.packet.pid;
            data_transmition.packet.pid = getpid();
            data_transmition.packet.value = 1;
            
            send_msg(msgid, (void *)&data_transmition, sizeof(data_transmition.packet), 0);
            next_device++;
            continue;
        } else if (data_received.msg_type == MESSAGE_KEY) {
            current_index = find_device(&data_received, devices);
            // Ignore if the device hasn't registered yet
            if (current_index == -1) { continue; }
            current_device = devices[current_index];
        }

        if (data_received.packet.type) { // Actuator
            if (!data_received.packet.value) {
                fprintf(stderr, "Actuator %s encountered an error.\n", current_device.name);
            }
            // Nothing else to do if yet recieve a message from an actuator.
            continue;
        } else { // Sensor
            // If the threshold hasn't been crossed or the device dosen't have
            // a partner then skip to the next message.
            if (data_received.packet.value < current_device.threshold) {
                printf("Received message from sensor pid: %d\n", data_received.packet.pid);
                continue;
            } else if (!current_device.has_partner) { continue; }

            // Send the data to the sensors partner.
            cpy_device_msg(&data_transmition, &current_device);
            data_transmition.msg_type = (long int) current_device.partner->pid;
            data_transmition.packet.value = 1;
        }

        printf("Received:");
        print_message(&data_received);
        printf("Sent:");
        print_message(&data_transmition);

    	send_msg(msgid, (void *)&data_transmition, sizeof(data_transmition.packet), 0);
    }

    exit(EXIT_SUCCESS);
}
