// Here's the client, client.c. The first part of this program opens the server FIFO,
// if it already exists, as a file. It then gets its own process ID, which forms some
// of the data that will be sent to the server. The client FIFO is created, ready for
// the next section.

#include "message.h"

int main(void) {
    int msgid;
    message_t my_data;
    message_t server_response;
    char buffer[BUFFER_SIZE];

    my_data.packet.pid = getpid();
    my_data.msg_type = 1;

    // Make sure queue is there
    msgid = msgget((key_t)1234, 0666 | IPC_CREAT);

    if (msgid == -1) {
        fprintf(stderr, "msgget failed with error: %d\n", errno);
        exit(EXIT_FAILURE);
    }

    printf("Enter some text: ");
    fgets(buffer, BUFFER_SIZE, stdin);
    strcpy(my_data.packet.text, buffer);

    if (msgsnd(msgid, (void *)&my_data, BUFFER_SIZE, 0) == -1) {
        fprintf(stderr, "msgsnd failed\n");
        exit(EXIT_FAILURE);
    }

    printf("%d\n%d\n%s\n", my_data.msg_type, my_data.packet.pid, my_data.packet.text);

    sleep(20);

    if (msgrcv(msgid, (void *)&server_response, BUFFER_SIZE,
               my_data.packet.pid, 0) == -1) {
        fprintf(stderr, "msgrcv failed with error: %d\n", errno);
        exit(EXIT_FAILURE);
    }
    printf("recieved: %s", server_response.packet.text);
    printf("%d\n%d\n%s\n", server_response.msg_type, server_response.packet.pid, server_response.packet.text);
    exit(EXIT_SUCCESS);
}
