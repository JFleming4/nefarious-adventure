#ifndef __MESSAGE_H__
#define __MESSAGE_H__

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <unistd.h>
#include <sys/msg.h>

#define BUFFER_SIZE 30
#define CLOUD_FIFO "/tmp/to_fluffy"

#define QUEUE_KEY 1234

#define MESSAGE_KEY 1
#define REGISTER_KEY 2
#define THRESHOLD_CROSS_KEY 3

#define SENSOR 0
#define ACTUATOR 1


// The standard packet sent in message from the devices and controller
typedef struct {
    pid_t pid;
    int value;
    int type;
    int threshold;
    char name[BUFFER_SIZE];
} packet_t;

// The standard message sent from the devices and controller
typedef struct {
    long int msg_type;
    packet_t packet;
} message_t;

struct device {
    pid_t pid;
    pid_t partner;
    int type;
    int value;
    int threshold_crossed;
    int threshold;
    char name[BUFFER_SIZE];
    char has_partner;
};

typedef struct device device_t;

typedef struct {
    pid_t pid;
    device_t sensor;
    device_t actuator;
    char event[BUFFER_SIZE];
} device_event_t;

typedef struct {
    long int msg_type;
    device_event_t packet;
} event_message_t;

void print_message(message_t *msg);
void print_event(event_message_t *msg);
void print_device(device_t *device);

void send_msg(int msgid, void *msg, size_t size, int block);
void receive_msg(int msgid, void *msg, size_t size, long int receive_type, int block);
void send_fifo(void *msg, const char *fifo, int mode, size_t size);
void receive_fifo(void *msg, const char *fifo, int mode, size_t size);

#endif
