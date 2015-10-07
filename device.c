#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/msg.h>

#include "message.h"

// When it is a sensor device this function will be called
void sensor(int pid, int msgid) {
    message_t sensor_data;  // struct to hold message for sending
    
    // This data won't change and is set outside the loop
    sensor_data.msg_type = MESSAGE_KEY;     // set message type
    sensor_data.packet.pid = pid;           // set pid
    sensor_data.packet.type = SENSOR;       // set it to type sensor
    
    while(1) {
        sensor_data.packet.value = rand() % 36; // Generate a random value 0 - 35
        send_msg(msgid, (void *)&sensor_data, sizeof(sensor_data.packet), IPC_NOWAIT); //send to server
        printf("Current Value: %d,from pid: %d\n", sensor_data.packet.value, pid); // print to console
        sleep(2);   //wait 2 seconds
    }
}

// When it is an actuator this function will be called
void actuator(int pid, int msgid, char *name) {
    message_t sensor_data;  // struct to hold server response
    
    while(1) {
        receive_msg(msgid, (void *)&sensor_data, sizeof(sensor_data.packet), (long int) pid,
        0); // wait for message from server
        printf("%s was turned on at pid: %d\n", name, pid);   // print a message to console
        sensor_data.packet.pid = pid;   // set pid
        sensor_data.packet.value = 1;   // set value to true
        sensor_data.packet.type = ACTUATOR; // set type to actuator
        send_msg(msgid, (void *)&sensor_data, sizeof(sensor_data.packet), IPC_NOWAIT);  // send back response to server
    }
}

int main(int argc, char *argv[]) {
    int msgid;  // msgid is used to locate the right message queue
    int pid = getpid(); // get the process id
    int type;   // sensor or actuator
    message_t sensor_data, server_response; // sensor or actuator

    if (argc < 4) {
        fprintf(stderr, "Not enough args, you gave %d, 4 were required!\n", argc);
        exit(EXIT_FAILURE);
    } else {    // copy the command line arguments to the sensor_data struct
        strcpy(sensor_data.packet.name, argv[1]);
        sscanf(argv[2], "%d", &sensor_data.packet.type);
        sscanf(argv[3], "%d", &sensor_data.packet.threshold);
    }
    
    type = sensor_data.packet.type; // save the type in case of corruption
    sensor_data.packet.pid = pid;   // set the pid
    sensor_data.msg_type = REGISTER_KEY;    // set this as a register message

    // Make sure queue is there
    msgid = msgget((key_t)1234, 0666);

    if (msgid == -1) {
        fprintf(stderr, "msgget failed with error: %d\n", errno);
        exit(EXIT_FAILURE);
    }
    
    // send the register message
    send_msg(msgid, (void *)&sensor_data, sizeof(sensor_data.packet), IPC_NOWAIT);
    
    // wait for an acknowladgement
    receive_msg(msgid, (void *)&server_response, sizeof(server_response.packet), (long int) pid, 0);
    
    // if the acknowladgement is false
    if(!server_response.packet.value) {
        fprintf(stderr, "the acknowladge failed for proccess: %d\n", pid);
        exit(EXIT_FAILURE);
    }
    
    printf("Sent:\n");
    print_message(&sensor_data);
    printf("Recieved:\n");
    print_message(&server_response);
    
    // check to see if this is an actuator or sensor
    if(type) {
        actuator(pid, msgid, sensor_data.packet.name);
    } else {
        sensor(pid, msgid);
    }
    
    
    exit(EXIT_SUCCESS);
}
