/* Por Prof. Daniel Batista <batista@ime.usp.br>
 * Em 27/8/2023
 * 
 * Um código simples de um servidor de eco a ser usado como base para
 * o EP1. Ele recebe uma linha de um cliente e devolve a mesma linha.
 * Teste ele assim depois de compilar:
 * 
 * ./redes-servidor-exemplo-ep1 8000
 * 
 * Com este comando o servidor ficará escutando por conexões na porta
 * 8000 TCP (Se você quiser fazer o servidor escutar em uma porta
 * menor que 1024 você precisará ser root ou ter as permissões
 * necessáfias para rodar o código com 'sudo').
 *
 * Depois conecte no servidor via telnet. Rode em outro terminal:
 * 
 * telnet 127.0.0.1 8000
 * 
 * Escreva sequências de caracteres seguidas de ENTER. Você verá que o
 * telnet exibe a mesma linha em seguida. Esta repetição da linha é
 * enviada pelo servidor. O servidor também exibe no terminal onde ele
 * estiver rodando as linhas enviadas pelos clientes.
 * 
 * Obs.: Você pode conectar no servidor remotamente também. Basta
 * saber o endereço IP remoto da máquina onde o servidor está rodando
 * e não pode haver nenhum firewall no meio do caminho bloqueando
 * conexões na porta escolhida.
 */

#include "amqp_queues.h"
#include "packages.h"
#include "amqp_message.h"
#include <pthread.h>

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <time.h>
#include <unistd.h>

#define LISTENQ 1
#define MAXDATASIZE 100

const int MAX_THREADS = 1e5;

ClientThread threads[MAX_THREADS];

void *handle_client(void *arg) {
    ClientThread *client = (ClientThread *)arg;

    /**** THREAD FILHA ****/
    printf("[Uma conexão aberta]\n");

    if (initialize_connection (client)) {
        printf("[Uma conexão fechada por falha na inicialização]\n");
        return NULL;
    }
    
    int _consume = discover_which_method (client);
    if (_consume == 1) {
        printf("[Uma conexão fechada por falha na descoberta do método básico]\n");
        return NULL;
    }

    {
        for (int i = 0; i < queueCount; i++) {
            printf ("\nqueue_names: %s\n", queues[i]->name);
            printf ("queue_clients: %d  queues_messages: %d \n", queues[i]->RR->client_count, queues[i]->RR->message_count);
            {
                AmqpClient* current = queues[i]->RR->front_client;
                if (current != NULL) {
                    do {
                        printf("connf: %d\n", current->connfd);
                        current = current->next;
                    } while (current != queues[i]->RR->front_client);
                }
            }
            {
                AmqpMessage* current = queues[i]->RR->front_message;
                if (current != NULL) {
                    while (current != NULL) {
                        printf("message_data: %s\n", current->data);
                        current = current->next;
                    }
                }
            }
        }

        printf ("\n\n");
    }

    if (!_consume && finish_connection (client)) {
        printf("[Uma conexão fechada por falha na finalização]\n");
        return NULL;
    }

    // printf ("queue_count: %d\n", queueCount);
    // for (int i = 0; i < queueCount; i++) printf ("queue_names: %s\n", queues[i]->name);
    // printf("[Uma conexão fechada]\n");

    return NULL;
}

int main (int argc, char **argv) {
    int listenfd, connfd;
    
    struct sockaddr_in servaddr;
   
    if (argc != 2) {
        fprintf(stderr,"Uso: %s <Porta>\n",argv[0]);
        fprintf(stderr,"Vai rodar um servidor de echo na porta <Porta> TCP\n");
        exit(1);
    }

    if ((listenfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("socket :(\n");
        exit(2);
    }

    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family      = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port        = htons(atoi(argv[1]));
    if (bind(listenfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) == -1) {
        perror("bind :(\n");
        exit(3);
    }

    if (listen(listenfd, LISTENQ) == -1) {
        perror("listen :(\n");
        exit(4);
    }

    printf("[Servidor no ar. Aguardando conexões na porta %s]\n",argv[1]);
    printf("[Para finalizar, pressione CTRL+c ou rode um kill ou killall]\n");
   
	for (int cur_thread = 0; ; cur_thread++) {
        if (cur_thread >= MAX_THREADS) {
            perror("Maximum number of threads reached :(\n");
            exit(7);
        }
        if ((connfd = accept(listenfd, (struct sockaddr *) NULL, NULL)) == -1 ) {
            perror("accept :(\n");
            exit(5);
        }

        threads[cur_thread].connfd = connfd;
        threads[cur_thread].client_output_queue = (ClientOutputQueue*)malloc(sizeof(ClientOutputQueue));
        if (pthread_create(&threads[cur_thread].T, NULL, handle_client, &threads[cur_thread]) != 0) {
            perror("pthread_create :(\n");
            exit(6);
        }
    }
    exit(0);
}
