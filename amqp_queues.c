// amqp_queues.c

#include "amqp_queues.h"
#include "amqp_message.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

int queueCount = 0;
AmqpQueue* queues[1024];
pthread_mutex_t queuesMutex;

void get_message_and_clients_count (const char* name, u_int32_t* message_count, u_int32_t* client_count) {
    pthread_mutex_lock(&queuesMutex);
    for (int i = 0; i < queueCount; i++) if (strcmp(queues[i]->name, name) == 0) {
        *message_count = queues[i]->RR->message_count;
        *client_count = queues[i]->RR->client_count;
        pthread_mutex_unlock(&queuesMutex);
        return;
    }
    pthread_mutex_unlock(&queuesMutex);
    assert (0 && "Queue not found when trying to get message and client count");
}

void declare_AmqpQueue(const char* name) {
    pthread_mutex_lock(&queuesMutex);
    
    for (int i = 0; i < queueCount; i++) if (strcmp(queues[i]->name, name) == 0) {
        pthread_mutex_unlock(&queuesMutex);
        return;
    }
    queues[queueCount] = (AmqpQueue*)malloc(sizeof(AmqpQueue));

    strcpy (queues[queueCount]->name, name);
    queues[queueCount]->RR = (RoundRobin*)malloc(sizeof(RoundRobin));
    
    queues[queueCount]->RR->front_message = queues[queueCount]->RR->rear_message = NULL;
    queues[queueCount]->RR->front_client = queues[queueCount]->RR->rear_client = NULL;

    if (pthread_mutex_init(&queues[queueCount]->queueMutex, NULL) != 0) {
        perror("Mutex initialization failed");
        pthread_mutex_unlock(&queuesMutex);
        exit(1);
    }

    queues[queueCount]->RR->message_count = 0;
    queues[queueCount]->RR->client_count = 0;
    queueCount++;

    pthread_mutex_unlock(&queuesMutex);
}

int add_client_to_AmqpQueue (const char* name, ClientThread* client_thread) {
    for (int i = 0; i < queueCount; i++) {
        if (strcmp(queues[i]->name, name) == 0) {
            pthread_mutex_lock(&queues[i]->queueMutex);
            add_client(queues[i]->RR, client_thread);
            pthread_mutex_unlock(&queues[i]->queueMutex);

            consume (queues[i]->RR, name);

            return 0;
        }
    }
    return 1;
    // deve retornar o erro para o cliente
    // basic.consume: server channel error 404, message: NOT_FOUND - no queue 'Hello' in vhost '/'
}

int rm_client_from_AmqpQueue (const char* name, int connfd) {
    for (int i = 0; i < queueCount; i++) {
        if (strcmp(queues[i]->name, name) == 0) {
            pthread_mutex_lock(&queues[i]->queueMutex);
            rm_client(queues[i]->RR, connfd);
            pthread_mutex_unlock(&queues[i]->queueMutex);
            return 0;
        }
    }
    assert (0 && "Queue not found when trying to remove client");
}

void publish_AmqpQueue (const char* name, const char* data) {
    for (int i = 0; i < queueCount; i++) {
        if (strcmp(queues[i]->name, name) == 0) {
            pthread_mutex_lock(&queues[i]->queueMutex);
            add_message(queues[i]->RR, data);
            pthread_mutex_unlock(&queues[i]->queueMutex);

            consume (queues[i]->RR, name);
            return;
        }
    }
    printf ("Queue not found\n");
    // se tentar publicar em uma fila que não existe, a mensagem é perdida
}