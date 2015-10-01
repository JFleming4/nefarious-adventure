#include <ctype.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>

#include <sys/msg.h>

#define BUFFER_SIZE 20

typedef struct {
    pid_t pid;
    char text[BUFFER_SIZE];
} packet_t;

typedef struct {
    long int msg_type;
    packet_t packet;
} message_t;
