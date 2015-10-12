#include "message.h"

void print_device(device_t *device) {
    // const char *type = (device->type) ? "ACTUATOR" : "SENSOR";
    const char *p = (device->has_partner) ? "TRUE" : "FALSE";
    printf(
        "Name: %s\nDevice Type: %d\nPID: %d\nValue: %d\nThreshold: %d\nPartnered: %s\n",
        device->name,
        device->type,
        device->pid,
        device->value,
        device->threshold,
        p
    );
}

void print_event(event_message_t *msg) {
    printf("PID: %d\nSensor:\n", msg->packet.pid);
    print_device(&msg->packet.sensor);
    printf("Actuator:\n");
    print_device(&msg->packet.actuator);
    printf("Event: %s\n", msg->packet.event);
}

void print_message(message_t *msg) {
    // const char *type = (msg->packet.type) ? "ACTUATOR" : "SENSOR";
    printf(
        "Msg Type: %ld\nName: %s\nDevice Type: %d\nPID: %d\nValue: %d\nThreshold: %d\n",
        msg->msg_type,
        msg->packet.name,
        msg->packet.type,
        msg->packet.pid,
        msg->packet.value,
        msg->packet.threshold
    );
}

void send_msg(int msgid, void *msg, size_t size, int block) {
    if (msgsnd(msgid, msg, size, block) == -1) {
        fprintf(stderr, "msgsnd failed: %d\n", errno);
        exit(EXIT_FAILURE);
    }
}

void receive_msg(int msgid, void *msg, size_t size, long int receive_type, int block) {
    if (msgrcv(msgid, msg, size, receive_type, block) == -1) {
        fprintf(stderr, "msgrcv failed with error: %d\n", errno);
        exit(EXIT_FAILURE);
    }
}


void send_fifo(void *msg, const char *fifo, int mode, size_t size) {
    int pipe, res;
    int bytes_sent = 0;
    if (access(fifo, mode) == -1) {
        fprintf(stderr, "Could not find %s\n", fifo);
    } else {
        pipe = open(fifo, mode);
        if (pipe != -1) {
            while(bytes_sent < size) {
                res = write(pipe, msg, PIPE_BUF);
                if (res == -1) {
                    fprintf(stderr, "Write error on pipe %s\n", fifo);
                }
                bytes_sent += res;
            }
            (void)close(pipe);
        } else {
            fprintf(stderr, "Open error on pipe %s\n", fifo);
        }
    }
}

void receive_fifo(void *msg, const char *fifo, int mode, size_t size) {
    int pipe, res;
    int bytes_read = 0;

    pipe = open(fifo, mode);

    if (pipe != -1) {
        do {
            res = read(pipe, msg, PIPE_BUF);
            bytes_read += res;
        } while (res > 0);
        (void)close(pipe);
    }
    else {
        printf("Process %d failed to open pipe %d\n", getpid(), pipe);
    }
}
