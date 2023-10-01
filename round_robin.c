#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "amqp_message.h"
#include "amqp_client.h"
#include "round_robin.h"

void add_client (RoundRobin* RR, int connfd, ClientOutputQueue* client_output_queue) {
    AmqpClient* new_client = (AmqpClient*)malloc(sizeof(AmqpClient));
    assert (new_client != NULL);


    new_client->connfd = connfd;
    new_client->client_output_queue = client_output_queue;
    new_client->next = NULL;

    if (RR->front_client == NULL) RR->front_client = RR->rear_client = new_client;
    else {
        RR->rear_client->next = new_client;
        RR->rear_client = new_client;
    }

    RR->rear_client->next = RR->front_client;
}

void rm_client (RoundRobin* RR, int connfd) {
    // temos que achar o client com o mesmo connfd
    // se so tem um client, temos que remover ele
    if (RR->front_client->next == RR->front_client) {
        if (RR->front_client->connfd == connfd) {
            free(RR->front_client);
            RR->front_client = NULL;
        }
        return;
    }

    AmqpClient* current = RR->front_client;
    do {
        if (current->connfd == connfd) {
            // achamos o client
            // temos que remover ele da lista
            if (current == RR->front_client) {
                RR->front_client = current->next;
                RR->rear_client->next = RR->front_client;
            } else {
                AmqpClient* prev = RR->front_client;
                while (prev->next != current) prev = prev->next;
                prev->next = current->next;

                if (current == RR->rear_client) RR->rear_client = prev;
            }
            free(current);
            return;
        }
        current = current->next;
    } while (current != RR->front_client);
}

void add_message (RoundRobin* RR, const char* data) {
    AmqpMessage* new_node = (AmqpMessage*)malloc(sizeof(AmqpMessage));
    assert (new_node != NULL);

    strncpy(new_node->data, data, sizeof(new_node->data) - 1);
    new_node->data[sizeof(new_node->data) - 1] = '\0'; // Null-terminate the string
    new_node->next = NULL;

    if (RR->rear_message == NULL) RR->front_message = RR->rear_message = new_node;
    else {
        RR->rear_message->next = new_node;
        RR->rear_message = new_node;
    }
}

void consume(RoundRobin* RR) {
    while (RR->front_message != NULL && RR->front_client != NULL) {
        // se temos pelo menos um cliente e uma mensagem
        char* data = strdup(RR->front_message->data);
        assert (data != NULL);

        AmqpMessage* temp = RR->front_message;
        RR->front_message = RR->front_message->next;

        if (RR->front_message == NULL) RR->rear_message = NULL;

        // temos que fazer isso ir para a droga do cliente
        // giramos o circulo
        AmqpClient* current = RR->front_client;
        RR->rear_client = current;
        RR->front_client = RR->front_client->next;

        add_message_to_client_output(current->client_output_queue, data);

        free(temp);
    }
}