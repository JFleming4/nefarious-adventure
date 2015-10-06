#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/msg.h>

#include "message.h"

int main(int argc, char *argv[]) {
    int msgid;
    int pid = getpid();
    int val;
    message_t sensor_data;
    message_t server_response, my_message;

    if (argc < 4) {
        fprintf(stderr, "Not enough args, you gave %d, 4 were required!\n", argc);
        exit(EXIT_FAILURE);
    } else {
        strcpy(sensor_data.packet.name, argv[1]);
        sscanf(argv[2], "%d", &sensor_data.packet.type);
        sscanf(argv[3], "%d", &sensor_data.packet.threshold);
    }

    sensor_data.packet.pid = pid;
    sensor_data.msg_type = REGISTER_KEY;

    // Make sure queue is there
    msgid = msgget((key_t)1234, 0666);

    if (msgid == -1) {
        fprintf(stderr, "msgget failed with error: %d\n", errno);
        exit(EXIT_FAILURE);
    }

    if (msgsnd(msgid, (void *)&sensor_data, sizeof(sensor_data.packet), IPC_NOWAIT) == -1) {
        fprintf(stderr, "msgsnd failed: %d\n", errno);
        exit(EXIT_FAILURE);
    }

    if (msgrcv(msgid, (void *)&server_response, sizeof(server_response.packet),
               pid, 0) == -1) {
        fprintf(stderr, "msgrcv failed with error: %d\n", errno);
        exit(EXIT_FAILURE);
    }
    
    if(!server_response.packet.value) {
        fprintf(stderr, "the acknowladge failed for proccess: %d\n", pid);
        exit(EXIT_FAILURE);
    }

    printf("Sent:\n");
    print_message(&sensor_data);
    printf("Recieved:\n");
    print_message(&server_response);
    
    my_message.msg_type = MESSAGE_KEY;
    my_message.packet.pid = pid;
    
    while(1) {
        val = rand() % 36;
        
        my_message.packet.value = val;
        
        if(msgsnd(msgid, (void *)&my_message, sizeof(my_message.packet), IPC_NOWAIT) == -1) {
            fprintf(stderr, "msgsnd failed: %d\n", errno);
            exit(EXIT_FAILURE);
        }
        sleep(2);
    }
    
    exit(EXIT_SUCCESS);
}
