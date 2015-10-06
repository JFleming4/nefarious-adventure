#include "message.h"

void print_message(message_t *msg) {
    const char *tmp = (msg->packet.type) ? "ACTUATOR" : "SENSOR";
    printf(
        "Type: %ld\nName: %s\nType: %s\nPID: %d\nValue: %d\nThreshold: %d\n",
        msg->msg_type,
        msg->packet.name,
        tmp,
        msg->packet.pid,
        msg->packet.value,
        msg->packet.threshold
    );
}
