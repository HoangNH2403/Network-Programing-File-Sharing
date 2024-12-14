#include "message.h"

#include <cstdio>
#include <cstring>

Message create_message(MessageType type, char* payload) {
    Message mess;
    mess.type = type;
    memcpy(mess.payload, payload, PAYLOAD_SIZE);
    mess.length = strlen(mess.payload);
    return mess;
}

Message create_status_message(MessageType type, Status status) {
    Message mess;
    mess.type = type;
    sprintf(mess.payload, "%s", status_str(status).c_str());
    mess.length = strlen(mess.payload);
    return mess;
}

void print_message(Message mess) {
    printf("Message: [%d - %d]: %s\n", mess.type, mess.length, mess.payload);
}

int messsagecpy(Message* mess, Message temp) {
    if (mess == NULL)
        return -1;
    mess->type = temp.type;
    mess->length = temp.length;
    memcpy(mess->payload, temp.payload, PAYLOAD_SIZE);
    return 0;
}