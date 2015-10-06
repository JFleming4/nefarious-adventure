#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/msg.h>

#include "message.h"

void sensor(int pid, int msgid) {
    message_t sensor_data;
    
    sensor_data.msg_type = MESSAGE_KEY;
    sensor_data.packet.pid = pid;
    
    while(1) {
        sensor_data.packet.value = rand() % 36;
        send_msg(msgid, (void *)&sensor_data, sizeof(sensor_data.packet), IPC_NOWAIT);
        printf("Current Value: %d\n", sensor_data.packet.value);
        sleep(2);
    }
}

int main(int argc, char *argv[]) {
    int msgid;
    int pid = getpid();
    message_t sensor_data, server_response;

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

    send_msg(msgid, (void *)&sensor_data, sizeof(sensor_data.packet), IPC_NOWAIT);
    
    receive_msg(msgid, (void *)&server_response, sizeof(server_response.packet), (long int) pid, 0);

    if(!server_response.packet.value) {
        fprintf(stderr, "the acknowladge failed for proccess: %d\n", pid);
        exit(EXIT_FAILURE);
    }
    
    printf("Sent:\n");
    print_message(&sensor_data);
    printf("Recieved:\n");
    print_message(&server_response);
    sensor(pid, msgid);
    
    
    exit(EXIT_SUCCESS);
}
