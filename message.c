#include "message.h"

void print_message(message_t *msg) {
    const char *type = (msg->packet.type) ? "ACTUATOR" : "SENSOR";
    printf(
        "Msg Type: %ld\nName: %s\nDevice Type: %s\nPID: %d\nValue: %d\nThreshold: %d\n",
        msg->msg_type,
        msg->packet.name,
        type,
        msg->packet.pid,
        msg->packet.value,
        msg->packet.threshold
    );
}

void send_msg(int msgid, void *msg, size_t size, int block) {
    if (msgsnd(msgid, msg, size, block) == -1) {
        fprintf(stderr, "msgsnd failed: %d\n", errno);
        exit(EXIT_FAILURE);
    }
}

void receive_msg(int msgid, void *msg, size_t size, long int receive_type, int block) {
    if (msgrcv(msgid, msg, size, receive_type, block) == -1) {
        fprintf(stderr, "msgrcv failed with error: %d\n", errno);
        exit(EXIT_FAILURE);
    }
}
