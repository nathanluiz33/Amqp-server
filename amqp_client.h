#include "packages.h"
#include "client_thread.h"

#ifndef AMQP_CLIENT_H
#define AMQP_CLIENT_H

typedef struct AmqpClient {
    ClientThread* client_thread;
    struct AmqpClient* next;
} AmqpClient;

int send_message_to_client(ClientThread* client, const char *queue_name, const char* data);

#endif