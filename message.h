#ifndef __MESSAGE_H__
#define __MESSAGE_H__

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <sys/msg.h>

#define BUFFER_SIZE 20

#define MESSAGE_KEY 1
#define REGISTER_KEY 2

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

void print_message(message_t *msg);
void send_msg(int msgid, void *msg, size_t size, int block);
void receive_msg(int msgid, void *msg, size_t size, long int receive_type, int block);

#endif
