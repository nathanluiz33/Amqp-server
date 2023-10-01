#include <string.h>
#include <stdio.h>
#include <assert.h>
#include <stdlib.h>

#include "amqp_client.h"


void add_message_to_client_output (ClientOutputQueue* queue, const char* data) {
    ClientOutput* new_node = (ClientOutput*)malloc(sizeof(ClientOutput));
    assert (new_node != NULL);

    strncpy(new_node->data, data, sizeof(new_node->data) - 1);
    new_node->data[sizeof(new_node->data) - 1] = '\0'; // Null-terminate the string
    new_node->next = NULL;

    if (queue->front == NULL) queue->front = queue->rear = new_node;
    else {
        queue->rear->next = new_node;
        queue->rear = new_node;
    }
}