#ifndef CLIENT_THREAD_H
#define CLIENT_THREAD_H

#include <stdio.h>
#include <stdlib.h>
#include <arpa/inet.h>

#define MAXLINE 4096

typedef struct ClientThread {
    int connfd;
    pthread_t T;
    ssize_t n;                                  // Armazena o tamanho da string lida do cliente
    char recvline[MAXLINE + 1];                 // Armazena linhas recebidas do cliente
    char sendline[MAXLINE + 1];                 // Armazena linhas a serem enviadas para o cliente
} ClientThread;

#endif