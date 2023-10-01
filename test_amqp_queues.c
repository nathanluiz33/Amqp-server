#include "amqp_queues.h"
// #include "amqp_message.h"
// #include "round_robin.h"

#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <assert.h>

void flush_clients (int n, ClientOutputQueue* client_output_queue[], int connfd[]) {
    for (int i = 0; i < n; i++) {
        printf ("client_output_queue[%d]:\n", i);
        while (client_output_queue[i]->front != NULL) {
            ClientOutput* current = client_output_queue[i]->front;
            printf("client_output_data: %s\n", current->data);
            client_output_queue[i]->front = client_output_queue[i]->front->next;
            free(current);
        }
        printf ("\n");
    }
}

int main () {
    assert (queueCount == 0);
    publish_AmqpQueue ("queue1", "data1");
    
    declare_AmqpQueue ("queue1");
    assert (queueCount == 1);

    publish_AmqpQueue ("queue1", "data1");

    declare_AmqpQueue ("queue2");
    publish_AmqpQueue ("queue2", "data2");

    assert (queueCount == 2);

    int n; scanf ("%d", &n);

    int connfd[n];
    ClientOutputQueue* client_output_queue[n];
    for (int i = 0; i < n; i++) {
        client_output_queue[i] = (ClientOutputQueue*)malloc(sizeof(ClientOutputQueue));
        connfd[i] = i;
        add_client_to_AmqpQueue ("queue2", connfd[i], client_output_queue[i]);
    }

    while (1) {
        char data[4096];
        scanf ("%s", data);
        if (strcmp(data, "end") == 0) break;
        publish_AmqpQueue ("queue2", data);
    }

    printf ("\nAFTER CONSUME\n");

    flush_clients (n, client_output_queue, connfd);

    for (int i = 0; i < n; i++) rm_client_from_AmqpQueue ("queue2", connfd[i]);


    printf ("\nAFTER RM_CLIENT\n");

    for (int i = n - 2; i >= 2; i--) add_client_to_AmqpQueue ("queue1", connfd[i], client_output_queue[i]);

    while (1) {
        char data[4096];
        scanf ("%s", data);
        if (strcmp(data, "end") == 0) break;
        publish_AmqpQueue ("queue1", data);
    }

    flush_clients (n, client_output_queue, connfd);
}