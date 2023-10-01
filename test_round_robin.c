#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "round_robin.h"

int main () {
    RoundRobin* RR;
    RR = (RoundRobin*)malloc(sizeof(RoundRobin));
    assert (RR->front_message == NULL && RR->rear_message == NULL && RR->front_client == NULL && RR->rear_client == NULL);

    int n; scanf ("%d", &n);

    int connfd[n];
    ClientOutputQueue* client_output_queue[n];
    for (int i = 0; i < n; i++) {
        client_output_queue[i] = (ClientOutputQueue*)malloc(sizeof(ClientOutputQueue));
        connfd[i] = i;
        add_client (RR, connfd[i], client_output_queue[i]);
    }

    while (1) {
        char data[4096];
        scanf ("%s", data);
        if (strcmp(data, "end") == 0) break;
        add_message (RR, data);
    }

    printf ("BEFORE CONSUME\n");
    {
        AmqpClient* current = RR->front_client;
        do {
            printf("connf: %d\n", current->connfd);
            current = current->next;
        } while (current != RR->front_client);
    }
    {
        AmqpMessage* current = RR->front_message;
        while (current != NULL) {
            printf("message_data: %s\n", current->data);
            current = current->next;
        }
    }
    consume (RR);

    printf ("\nAFTER CONSUME\n");

    {
        AmqpClient* current = RR->front_client;
        do {
            printf("connf: %d\n", current->connfd);
            current = current->next;
        } while (current != RR->front_client);
    }
    {
        AmqpMessage* current = RR->front_message;
        while (current != NULL) {
            printf("message_data: %s\n", current->data);
            current = current->next;
        }
    }

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

    rm_client (RR, 0);
    rm_client (RR, 4);
    rm_client (RR, 2);
    printf ("\nAFTER RM\n");
    {
        AmqpClient* current = RR->front_client;
        do {
            printf("connf: %d\n", current->connfd);
            current = current->next;
        } while (current != RR->front_client);
    }

    add_message (RR, "ola");
    add_message (RR, "ok");
    add_message (RR, "blz");

    consume (RR);

    printf ("\nAFTER CONSUME\n");

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