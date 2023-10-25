// amqp_queues.h
#ifndef AMQP_QUEUES_H
#define AMQP_QUEUES_H

#include <string.h>
#include <stdio.h>
#include "round_robin.h"

typedef struct AmqpQueue {
    char name[256];
    RoundRobin* RR;
} AmqpQueue;

void declare_AmqpQueue(const char* name);
int add_client_to_AmqpQueue (const char* name, ClientThread* client_thread);
void publish_AmqpQueue (const char* name, const char* data);
int rm_client_from_AmqpQueue (const char* name, int connfd);
void get_message_and_clients_count (const char* name, u_int32_t* message_count, u_int32_t* client_count);

extern int queueCount;
extern AmqpQueue* queues[1024];

#endif