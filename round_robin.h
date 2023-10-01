#include <string.h>
#include <stdio.h>
#include "amqp_message.h"
#include "amqp_client.h"

#ifndef ROUND_ROBIN_H
#define ROUND_ROBIN_H

typedef struct RoundRobin {
    // temos uma fila de mensagens a serem enviadas
    AmqpMessage* front_message;
    AmqpMessage* rear_message;
    // temos uma fila de clientes a receber as mensagens
    AmqpClient* front_client;
    AmqpClient* rear_client;
} RoundRobin;

void add_client (RoundRobin *RR, int connfd, ClientOutputQueue* client_output_queue);
void rm_client (RoundRobin* RR, int connfd);
void add_message (RoundRobin* RR, const char* data);
void consume(RoundRobin* RR);

#endif