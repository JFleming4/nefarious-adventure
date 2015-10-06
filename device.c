#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/msg.h>

#include "message.h"

int main(int argc, char *argv[]) {
    int msgid;
    message_t my_data, server_response;

    if (argc < 4) {
        fprintf(stderr, "Not enough args, you gave %d, 4 were required!\n", argc);
        exit(EXIT_FAILURE);
    } else {
        strcpy(my_data.packet.name, argv[1]);
        sscanf(argv[2], "%d", &my_data.packet.type);
        sscanf(argv[3], "%d", &my_data.packet.threshold);
    }

    my_data.packet.pid = getpid();
    my_data.msg_type = 1;

    // Make sure queue is there
    msgid = msgget((key_t)1234, 0666);

    if (msgid == -1) {
        fprintf(stderr, "msgget failed with error: %d\n", errno);
        exit(EXIT_FAILURE);
    }

    if (msgsnd(msgid, (void *)&my_data, sizeof(my_data.packet), IPC_NOWAIT) == -1) {
        fprintf(stderr, "msgsnd failed: %d\n", errno);
        exit(EXIT_FAILURE);
    }

    if (msgrcv(msgid, (void *)&server_response, sizeof(my_data.packet),
               my_data.packet.pid, 0) == -1) {
        fprintf(stderr, "msgrcv failed with error: %d\n", errno);
        exit(EXIT_FAILURE);
    }

    printf("Sent:\n");
    print_message(&my_data);
    printf("Recieved:\n");
    print_message(&server_response);
    exit(EXIT_SUCCESS);
}
