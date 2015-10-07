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
    int value;
    int threshold_crossed;
    int threshold;
    char name[BUFFER_SIZE];
    char has_partner;
    struct device *partner;
};

typedef struct device device_t;

static pid_t child_pid;
static int msgid, next_device;
static device_t devices[MAX_DEVICES];


/*
 * Signal Handlers
 */

void cleanup_parent(int sig) {
    kill(child_pid, SIGTERM);
    msgctl(msgid, IPC_RMID, 0);
    exit(EXIT_SUCCESS);
}

void cleanup_child(int sig) {
    for (int i = 0; i < next_device; i++) {
        kill(devices[i].pid, SIGTERM);
    }
    exit(EXIT_SUCCESS);
}

void threshold_crossing(int sig) {
    message_t msg;
    receive_msg(msgid, (void *)&msg, sizeof(packet_t), THRESHOLD_CROSS_KEY, IPC_NOWAIT);
    printf(
        "Sensor %d crossed threshold %d with a value of %d and actuator %d was turned %s.\n",
        msg.packet.pid,
        msg.packet.threshold,
        msg.packet.value,
        msg.packet.type,
        msg.packet.name
    );
}

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
    message_t transmit;
    device_t *device = &devices[next_device];

    device->pid = (long int) msg->packet.pid;
    device->type = msg->packet.type;
    device->threshold = msg->packet.threshold;
    device->threshold_crossed = 0;
    strcpy(device->name, msg->packet.name);
    printf("msg name: %s\ndevice name: %s\n", msg->packet.name, device->name);

    // Setup the data to send back to acknowledge the registration.
    strcpy(transmit.packet.name, device->name);
    transmit.packet.threshold = device->threshold;
    transmit.packet.type = device->type;
    transmit.msg_type = (long int) device->pid;
    transmit.packet.pid = getpid();
    transmit.packet.value = 1;

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

                printf("2 Device: %d, is partnered with: %d\n", devices[i].pid, devices[i].partner -> pid);
            }
        }
    }
    send_msg(msgid, (void *)&transmit, sizeof(packet_t), 0);
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

void actuator(message_t *msg, device_t *device) {
    message_t transmit;
    device->value = msg->packet.value;

    if (!msg->packet.value) {
        fprintf(stderr, "Actuator %s encountered an error.\n", device->name);
    }

    // Transfer the sensor data to the msg and populate type with the
    // actuator pid, then send a special message to the parent and
    // signal to inform it that it's there.
    strcpy(transmit.packet.name, msg->packet.name);
    transmit.packet.threshold = device->partner->threshold;
    transmit.packet.type = device->pid;
    transmit.msg_type = THRESHOLD_CROSS_KEY;
    transmit.packet.pid = device->partner->pid;
    transmit.packet.value = device->partner->value;
    send_msg(msgid, (void *)&transmit, sizeof(packet_t), 0);
    kill(getppid(), SIGUSR1);
}

void sensor(message_t *received, device_t *device) {
    message_t transmit;
    // Save the last piece of sensor data.
    device->value = received->packet.value;

    if (received->packet.value < device->threshold) {
        // Detect if you fell below the threshold continue if you didn't
        if (!device->threshold_crossed) { return; }

        device->threshold_crossed = 0;
        transmit.packet.value = 0;
    } else {
        // Detect if you rose above the threshold continue if you didn't
        if (device->threshold_crossed) { return; }

        device->threshold_crossed = 1;
        transmit.packet.value = 1;
    }

    transmit.msg_type = (long int) device->partner->pid;
    send_msg(msgid, (void *)&transmit, sizeof(packet_t), 0);
}

void child(void) {
    message_t data_received;
    device_t *current_device; //, devices[MAX_DEVICES];
    next_device = 0;
    int current_index;
    struct sigaction cleanup;

    cleanup.sa_handler = cleanup_child;
    sigemptyset(&cleanup.sa_mask);
    cleanup.sa_flags = 0;

    sigaction(SIGTERM, &cleanup, 0);

    for (int i = 0; i < MAX_DEVICES; i++) {
        devices[i].has_partner = 0;
    }

    while(1) {

        receive_msg(msgid, (void *)&data_received, sizeof(packet_t), RECEIVE_TYPE, 0);

        // Registration vs Message Section
        if (data_received.msg_type == REGISTER_KEY) {
            register_device(&data_received, devices, next_device++);
            continue;
        } else if (data_received.msg_type == MESSAGE_KEY) {
            current_index = find_device(&data_received, devices);

            // Ignore if the device hasn't registered yet
            if (current_index == -1) { continue; }
            current_device = &devices[current_index];
        }
        // If the device dosen't have a partner then skip to the next message.
        if (!current_device->has_partner) { continue; }

        // Message Recieved From Device
        if (data_received.packet.type) { // Actuator
            actuator(&data_received, current_device);
        } else { // Sensor
            sensor(&data_received, current_device);
        }
    }
}

void parent(void) {
    struct sigaction cleanup, msg_received;

    cleanup.sa_handler = cleanup_parent;
    sigemptyset(&cleanup.sa_mask);
    cleanup.sa_flags = 0;

    msg_received.sa_handler = threshold_crossing;
    sigemptyset(&msg_received.sa_mask);
    msg_received.sa_flags = 0;

    sigaction(SIGTERM, &cleanup, 0);
    sigaction(SIGINT, &cleanup, 0);
    sigaction(SIGUSR1, &msg_received, 0);

    while(1);

    exit(EXIT_SUCCESS);
}


int main(void) {
    // Setup the message queue
    msgid = msgget((key_t) 1234, 0666 | IPC_CREAT);
    if (msgid == -1) {
    	fprintf(stderr, "msgget failed with error: %d\n", errno);
    	exit(EXIT_FAILURE);
    }

    child_pid = fork();

    switch(child_pid) {
    case -1:
        perror("fork failed");
        exit(EXIT_FAILURE);
    case 0:
        printf("Child: %d\n", getpid());
        child();
        break;
    default:
        printf("Parent: %d\n", getpid());
        parent();
        break;
    }

    exit(EXIT_SUCCESS);
}
