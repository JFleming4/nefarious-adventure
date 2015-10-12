#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <limits.h>
#include <signal.h>
#include <sys/msg.h>
#include <sys/stat.h>

#include "message.h"

#define TO_PARENT "/tmp/to_parent"
#define FROM_PARENT "/tmp/from_parent"
#define MAX_DEVICES 3

static pid_t child_pid;
static pid_t controllers[MAX_DEVICES];
static int next_device;

void cleanup_parent(int sig) {
    unlink(CLOUD_FIFO);
    unlink(TO_PARENT);
    unlink(FROM_PARENT);
    kill(child_pid, SIGTERM);
    exit(EXIT_SUCCESS);
}

void cleanup_child(int sig) {
    for (int i = 0; i < next_device; i++) {
        kill(controllers[i], SIGTERM);
    }
    exit(EXIT_SUCCESS);
}

void child(void) {
    event_message_t data_received;
    struct sigaction cleanup;

    cleanup.sa_handler = cleanup_child;
    sigemptyset(&cleanup.sa_mask);
    cleanup.sa_flags = 0;

    sigaction(SIGTERM, &cleanup, 0);

    while (1) {
        receive_fifo(&data_received, CLOUD_FIFO, O_RDONLY, sizeof(event_message_t));

        if (data_received.msg_type == REGISTER_KEY) {
            controllers[next_device++] = data_received.packet.pid;
            printf("Controller %d was Registered.\n", data_received.packet.pid);
            continue;
        }

        printf(
            "Sensor %s crossed threshold %d with a value of %d and actuator %s was turned %s.\n",
            data_received.packet.sensor.name,
            data_received.packet.sensor.threshold,
            data_received.packet.sensor.value,
            data_received.packet.actuator.name,
            data_received.packet.event
        );
    }
}

void parent(void) {
    struct sigaction cleanup;

    cleanup.sa_handler = cleanup_parent;
    sigemptyset(&cleanup.sa_mask);
    cleanup.sa_flags = 0;

    sigaction(SIGTERM, &cleanup, 0);
    sigaction(SIGINT, &cleanup, 0);

    while(1);
}

void make_fifo(const char *fifo) {
    if (access(fifo, F_OK) == -1) {
        if (mkfifo(fifo, 0777) != 0) {
            fprintf(stderr, "Could not create fifo %s\n", fifo);
            exit(EXIT_FAILURE);
        }
    }
}

int main(int argc, char *argv[]) {
    next_device = 0;

    make_fifo(CLOUD_FIFO);
    make_fifo(TO_PARENT);
    make_fifo(FROM_PARENT);

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
