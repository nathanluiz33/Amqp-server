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

void handle_publish (ClientThread *client, amqp_protocol_package* package) {

}

void send_declare_queue_ok (ClientThread *client, char *queue_name) {
    printf ("aquiqueue_name: %s\n", queue_name);

    amqp_protocol_package* package = (amqp_protocol_package*)malloc(sizeof(amqp_protocol_package));
    package->type = 1;
    package->channel = 1;
    package->class_id = CLASS_QUEUE;
    package->method_id = METHOD_DECLARE_OK;

    int cur = 11;
    u_int8_t queue_size = strlen (queue_name);

    // adicionamos o tamanho do nome da fila
    memcpy (client->sendline + cur, &queue_size, 1);
    cur++;

    // colocamos o nome da fila
    memcpy (client->sendline + cur, queue_name, queue_size);
    cur += queue_size;

    u_int32_t message_count = 0, consumer_count = 0;
    get_message_and_clients_count (queue_name, &message_count, &consumer_count);

    message_count = htonl (message_count);
    consumer_count = htonl (consumer_count);

    memcpy (client->sendline + cur, &message_count, 4);
    cur += 4;

    memcpy (client->sendline + cur, &consumer_count, 4);
    cur += 4;

    package->payload_size = cur - 7;

    unparse_package (package);
    memcpy (client->sendline, package, sizeof (*package));

    client->sendline[cur] = '\xce';

    write (client->connfd, client->sendline, cur + 1);

    printf ("declare queue ok sent!\n");
}

void handle_queue_declare (ClientThread *client, amqp_protocol_package* package) {
    char queue_name[256];

    int size = client->recvline[14];
    memcpy (queue_name, client->recvline + 14, size);

    declare_AmqpQueue (queue_name);

    send_declare_queue_ok (client, queue_name);
}

int discover_which_method (ClientThread *client) {
    if (read_next_method (client)) return 1;

    amqp_protocol_package* package = (amqp_protocol_package*)malloc(sizeof(amqp_protocol_package));
    memcpy (package, client->recvline, sizeof (*package));

    unparse_package (package);
    
    if (package->type != TYPE_METHOD || package->channel != 1) return 1;
    
    if (package->class_id == CLASS_BASIC && package->method_id == METHOD_PUBLISH) {
        printf ("package: %d %d %d %d %d\n", package->type, package->channel, package->payload_size, package->class_id, package->method_id);
        handle_publish (client, package);
    }
    else if (package->class_id == CLASS_BASIC && package->method_id == METHOD_CONSUME) {

    }
    else if (package->class_id == CLASS_QUEUE && package->method_id == METHOD_DECLARE) {
        handle_queue_declare (client, package);
    }
    else {
        printf ("Metodo nao suportado pelo servidor\n");
        return 1;
    }

    return 0;
}

int finish_connection (ClientThread *client) {
    if (read_channel_close (client) != 0) {
        printf("[Uma conexão fechada por channel_close incorreto]\n");
        return 1;
    }
    send_method_brute (client, channel_close_ok_brute, channel_close_ok_brute_size);
    if (read_connection_close (client) != 0) {
        printf("[Uma conexão fechada por connection_close incorreto]\n");
        return 1;
    }
    send_method_brute (client, connection_close_ok_brute, connection_close_ok_brute_size);
    return 0;
}

void *handle_client(void *arg) {
    ClientThread *client = (ClientThread *)arg;

    /**** THREAD FILHA ****/
    printf("[Uma conexão aberta]\n");

    if (initialize_connection (client)) {
        printf("[Uma conexão fechada por falha na inicialização]\n");
        return NULL;
    }
    
    if (discover_which_method (client)) {
        printf("[Uma conexão fechada por falha na descoberta do método básico]\n");
        return NULL;
    }

    if (finish_connection (client)) {
        printf("[Uma conexão fechada por falha na finalização]\n");
        return NULL;
    }

    printf ("queue_count: %d\n", queueCount);
    for (int i = 0; i < queueCount; i++) printf ("queue_names: %s\n", queues[i]->name);
    printf("[Uma conexão fechada]\n");

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
