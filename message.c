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
