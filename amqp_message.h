#ifndef AMQP_MESSAGE_H
#define AMQP_MESSAGE_H

typedef struct AmqpMessage {
    char data[4096];
    struct AmqpMessage* next;
} AmqpMessage;

#endif