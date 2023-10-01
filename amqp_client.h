#ifndef AMQP_CLIENT_H
#define AMQP_CLIENT_H

typedef struct ClientOutput {
    char data[4096];
    struct ClientOutput* next;
} ClientOutput;

typedef struct ClientOutputQueue {
    ClientOutput* front;
    ClientOutput* rear;
} ClientOutputQueue;

typedef struct AmqpClient {
    int connfd;
    ClientOutputQueue* client_output_queue;
    struct AmqpClient* next;
} AmqpClient;

void add_message_to_client_output(ClientOutputQueue* queue, const char* data);

#endif