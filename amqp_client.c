#include <string.h>
#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <pthread.h>

#include "amqp_client.h"

int send_message_to_client (ClientThread* client, const char *queue_name, const char* data) {
    // aqui ja tentamos mandar a mensagem para o cliente
    // se nao conseguirmos, devemos retornar 1
    pthread_mutex_lock(&client->clientMutex);
    handle_deliver (client, queue_name, data);
    pthread_mutex_unlock(&client->clientMutex);
    
    pthread_mutex_lock(&client->clientMutex);
    if (read_next_method (client)) {
        rm_client_from_AmqpQueue (queue_name, client->connfd);
        pthread_mutex_unlock(&client->clientMutex);
        return 1;
    }


    method_payloads_header* method_payloads = (method_payloads_header*)malloc(sizeof(method_payloads_header));
    memcpy (method_payloads, client->recvline + sizeof (general_frame_header), sizeof (*method_payloads));
    unparse_method_payloads_header (method_payloads);
    if (method_payloads->class_id != CLASS_BASIC || method_payloads->method_id != METHOD_ACK) printf ("ACK invalido\n");

    free (method_payloads);

    pthread_mutex_unlock(&client->clientMutex);

    return 0;
}