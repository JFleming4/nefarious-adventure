#ifndef __MESSAGE_H__
#define __MESSAGE_H__

#include <stdlib.h>
#include <stdio.h>

#define BUFFER_SIZE 20
#define SENSOR 0
#define ACTUATOR 1

typedef struct {
    pid_t pid;
    int type;
    int value;
    int threshold;
    char name[BUFFER_SIZE];
} packet_t;

typedef struct {
    long int msg_type;
    packet_t packet;
} message_t;

void print_message(message_t *msg);

#endif
