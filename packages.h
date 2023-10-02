#include <stdio.h>
#include <arpa/inet.h>
#include <unistd.h>
#include "hardcoded_values.h"
#include "amqp_queues.h"

#define FRAME_HEADER_SIZE 7
#define MAXLINE 4096

typedef struct ClientThread {
    int connfd;
    pthread_t T;
    ClientOutputQueue* client_output_queue;
    ssize_t n;                                  // Armazena o tamanho da string lida do cliente
    char recvline[MAXLINE + 1];                 // Armazena linhas recebidas do cliente
    char sendline[MAXLINE + 1];                 // Armazena linhas a serem enviadas para o cliente
} ClientThread;

int read_protocol_header (ClientThread *client);

int read_next_method (ClientThread *client);

void send_method_brute (ClientThread *client, u_int8_t hardcoded[], size_t size);

void send_connection_start (ClientThread *client);

int read_connection_start_ok (ClientThread *client);

void send_connection_tune (ClientThread *client);

int read_connection_tune_ok (ClientThread *client);

int read_connection_open (ClientThread *client);

void send_connection_open_ok (ClientThread *client);

int read_channel_open (ClientThread *client);

int read_channel_close (ClientThread *client);

int read_connection_close (ClientThread *client);

void send_channel_open_ok (ClientThread *client);

int initialize_connection (ClientThread *client);