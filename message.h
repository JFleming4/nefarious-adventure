#ifndef __MESSAGE_H__
#define __MESSAGE_H__

#include <stdlib.h>
#include <stdio.h>

#define BUFFER_SIZE 20

#define MESSAGE_KEY 1
#define REGISTER_KEY 2

#define SENSOR 0
#define ACTUATOR 1

// The Data used to Register a device with the controller
typedef struct {
    pid_t pid;
    int type;
    int threshold;
    char name[BUFFER_SIZE];
} device_data_t;

// The standard packet sent in message from the devices and controller
typedef struct {
    pid_t pid;
    int value;
} packet_t;

// The registration message sent by the devices
typedef struct {
    long int msg_type;
    device_data_t device_data;
} register_message_t;

// The standard message sent from the devices and controller
typedef struct {
    long int msg_type;
    packet_t packet;
} device_t;

void print_message(message_t *msg);

#endif
