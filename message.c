#include "message.h"

void print_register_message(register_message_t *msg) {
    const char *type = (msg->device_data.type) ? "ACTUATOR" : "SENSOR";
    printf(
        "Msg Type: %ld\nName: %s\nDevice Type: %s\nPID: %d\nThreshold: %d\n",
        msg->msg_type,
        msg->device_data.name,
        type,
        msg->device_data.pid,
        msg->device_data.threshold
    );
}


void print_device_message(device_message_t *msg) {
    printf(
        "Msg Type: %ld\nPID: %d\nValue: %d\n",
        msg->msg_type,
        msg->packet.pid,
        msg->packet.value
    );
}
