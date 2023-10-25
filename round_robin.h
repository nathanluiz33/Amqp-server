#ifndef ROUND_ROBIN_H
#define ROUND_ROBIN_H

#include <string.h>
#include <stdio.h>
#include "amqp_message.h"
#include "amqp_client.h"

typedef struct RoundRobin {
    // temos uma fila de mensagens a serem enviadas
    u_int32_t message_count;
    AmqpMessage* front_message;
    AmqpMessage* rear_message;
    // temos uma fila de clientes a receber as mensagens
    u_int32_t client_count;
    AmqpClient* front_client;
    AmqpClient* rear_client;
} RoundRobin;

void add_client (RoundRobin *RR, ClientThread* client_thread);
void rm_client (RoundRobin* RR, int connfd);
void add_message (RoundRobin* RR, const char* data);
void consume(RoundRobin* RR, const char* queue_name);

#endif