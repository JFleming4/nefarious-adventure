#include <ctype.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <fcntl.h>
#include <limits.h>
#include <sys/msg.h>
#include <sys/shm.h>
#include <sys/stat.h>

#include "message.h"

#define RECEIVE_TYPE -2L
#define MAX_DEVICES 8

static pid_t child_pid;
static int msgid, shmid, next_device;
static device_t devices[MAX_DEVICES];
static void *shared_mem;


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

// Returns the index of the device that matches the sender of the message.
// Returns -1 if it's not found.
int find_device(pid_t pid) {
    for (int i = 0; i < next_device; i++) {
        if (devices[i].pid == pid) {
            return i;
        }
    }
    return -1;
}

// Transfers data from sensor to cloud to notify of threshold crossings.
void threshold_crossing(int sig) {
    event_message_t *msg = (event_message_t *) shared_mem;

    msg->packet.pid = getpid();

    send_fifo(msg, CLOUD_FIFO, O_WRONLY | O_NONBLOCK, sizeof(event_message_t));
}

// Register a device with the controller by adding it to the devices array.
void register_device(message_t *msg, int next_device) {
    message_t transmit;

    devices[next_device].pid = (long int) msg->packet.pid;
    devices[next_device].type = msg->packet.type;
    devices[next_device].threshold = msg->packet.threshold;
    devices[next_device].threshold_crossed = 0;
    strcpy(devices[next_device].name, msg->packet.name);

    // Setup the data to send back to acknowledge the registration.
    strcpy(transmit.packet.name, devices[next_device].name);
    transmit.packet.threshold = devices[next_device].threshold;
    transmit.packet.type = devices[next_device].type;
    transmit.msg_type = (long int) devices[next_device].pid;
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
                devices[i].partner = devices[next_device].pid;
                devices[next_device].has_partner = 1;
                devices[next_device].partner = devices[i].pid;

                printf("Device: %d, is partnered with: %d\n", devices[next_device].pid, devices[next_device].partner);
            }
        }
    }
    send_msg(msgid, (void *)&transmit, sizeof(packet_t), 0);
}

// Sends signal to parent process to notfiy that an actuators state has
// changed
void actuator(message_t *msg, int actuator) {
    event_message_t *transmit;
    int sensor = find_device(devices[actuator].partner);
    devices[actuator].value = msg->packet.value;

    if (!msg->packet.value) {
        fprintf(stderr, "Actuator %s encountered an error.\n", devices[actuator].name);
    }

	transmit = (event_message_t *) shared_mem;

    // Transfer the sensor data to the msg and populate type with the
    // actuator pid, then send a special message to the parent and
    // signal to inform it that it's there.

    transmit->msg_type = THRESHOLD_CROSS_KEY;

    transmit->packet.sensor.pid = devices[sensor].pid;
    transmit->packet.sensor.type = devices[sensor].type;
    transmit->packet.sensor.value = devices[sensor].value;
    transmit->packet.sensor.threshold = devices[sensor].threshold;
    strcpy(transmit->packet.sensor.name, devices[sensor].name);

    transmit->packet.actuator.pid = devices[actuator].pid;
    transmit->packet.actuator.type = devices[actuator].type;
    transmit->packet.actuator.value = devices[actuator].value;
    transmit->packet.actuator.threshold = devices[actuator].threshold;
    strcpy(transmit->packet.actuator.name, devices[actuator].name);

    strcpy(transmit->packet.event, msg->packet.name);

    kill(getppid(), SIGUSR1);
}

// Receives Sensor data and stores it in the appropriate device record.
void sensor(message_t *received, int sensor) {
    message_t transmit;
    // Save the last piece of sensor data.
    devices[sensor].value = received->packet.value;

    if (received->packet.value < devices[sensor].threshold) {
        // Detect if you fell below the threshold continue if you didn't
        if (!devices[sensor].threshold_crossed) { return; }

        devices[sensor].threshold_crossed = 0;
        transmit.packet.value = 0;
    } else {
        // Detect if you rose above the threshold continue if you didn't
        if (devices[sensor].threshold_crossed) { return; }

        devices[sensor].threshold_crossed = 1;
        transmit.packet.value = 1;
    }

    transmit.msg_type = (long int) devices[sensor].partner;
    send_msg(msgid, (void *)&transmit, sizeof(packet_t), 0);
}

// The procedure for the child process.
void child(void) {
    message_t data_received;
    next_device = 0;
    int current_device;
    struct sigaction cleanup;

    cleanup.sa_handler = cleanup_child;
    sigemptyset(&cleanup.sa_mask);
    cleanup.sa_flags = 0;

    sigaction(SIGTERM, &cleanup, 0);

	shared_mem = shmat(shmid, (void *) 0, 0);
	if (shared_mem == (void *) -1) {
		fprintf(stderr, "shmat failed\n");
		exit(EXIT_FAILURE);
	}

	printf("Memory attached at %X\n", (long int)shared_mem);

    for (int i = 0; i < MAX_DEVICES; i++) {
        devices[i].has_partner = 0;
    }

    while(1) {

        receive_msg(msgid, (void *)&data_received, sizeof(packet_t), RECEIVE_TYPE, 0);

        // Registration vs Message Section
        if (data_received.msg_type == REGISTER_KEY) {
            register_device(&data_received, next_device++);
            continue;
        } else if (data_received.msg_type == MESSAGE_KEY) {
            current_device = find_device(data_received.packet.pid);
            // Ignore if the device hasn't registered yet
            if (current_device == -1) {
                fprintf(stderr, "Could not find device: %d\n", data_received.packet.pid);
                continue;
            }
        }
        // If the device dosen't have a partner then skip to the next message.
        if (!devices[current_device].has_partner) { continue; }

        // Message Recieved From Device
        if (data_received.packet.type) { // Actuator
            actuator(&data_received, current_device);
        } else { // Sensor
            sensor(&data_received, current_device);
        }
    }
}

// The procedure for the parent process.
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
	
	shared_mem = shmat(shmid, (void *) 0, 0);
	if (shared_mem == (void *) -1) {
		fprintf(stderr, "shmat failed\n");
		exit(EXIT_FAILURE);
	}

	printf("Memory attached at %X\n", (long int)shared_mem);

    while(1);

    exit(EXIT_SUCCESS);
}

int main(int argc, char *argv[]) {
    event_message_t event_msg;
	shared_mem = (void *) 0;	

    event_msg.msg_type = REGISTER_KEY;
    event_msg.packet.pid = getpid();
    send_fifo(&event_msg, CLOUD_FIFO, O_WRONLY | O_NONBLOCK, sizeof(event_message_t));

    // Setup the message queue
    msgid = msgget((key_t) 1234, 0666 | IPC_CREAT);
    if (msgid == -1) {
    	fprintf(stderr, "msgget failed with error: %d\n", errno);
    	exit(EXIT_FAILURE);
    }

	shmid = shmget((key_t) 4321, sizeof(device_event_t), 0666 | IPC_CREAT);
	if (shmid == -1) {
		fprintf(stderr, "shmget faild!\n");
		exit(EXIT_FAILURE);
	}

    child_pid = fork();

    switch(child_pid) {
    case -1:
        perror("fork failed\n");
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
