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
#define MAXLINE 4096

const int MAX_THREADS = 1e5;

typedef struct ClientThread {
    int connfd;
    pthread_t T;
    ClientOutputQueue* client_output_queue;
    ssize_t n;                                  // Armazena o tamanho da string lida do cliente
    char recvline[MAXLINE + 1];                 // Armazena linhas recebidas do cliente
} ClientThread;

ClientThread threads[MAX_THREADS];

// uint8_t connection_start [] = {0x01, 0x00, 0x00, 0x00, 0x00, 0x01, 0xf9, 0x00, 0x0a, 0x00, 0x0a, 0x00, 0x09, 0x00, 0x00, 0x01, 0xd4, 0x0c, 0x63, 0x61, 0x70, 0x61, 0x62, 0x69, 0x6c, 0x69, 0x74, 0x69, 0x65, 0x73, 0x46, 0x00, 0x00, 0x00, 0xc7, 0x12, 0x70, 0x75, 0x62, 0x6c, 0x69, 0x73, 0x68, 0x65, 0x72, 0x5f, 0x63, 0x6f, 0x6e, 0x66, 0x69, 0x72, 0x6d, 0x73, 0x74, 0x01, 0x1a, 0x65, 0x78, 0x63, 0x68, 0x61, 0x6e, 0x67, 0x65, 0x5f, 0x65, 0x78, 0x63, 0x68, 0x61, 0x6e, 0x67, 0x65, 0x5f, 0x62, 0x69, 0x6e, 0x64, 0x69, 0x6e, 0x67, 0x73, 0x74, 0x01, 0x0a, 0x62, 0x61, 0x73, 0x69, 0x63, 0x2e, 0x6e, 0x61, 0x63, 0x6b, 0x74, 0x01, 0x16, 0x63, 0x6f, 0x6e, 0x73, 0x75, 0x6d, 0x65, 0x72, 0x5f, 0x63, 0x61, 0x6e, 0x63, 0x65, 0x6c, 0x5f, 0x6e, 0x6f, 0x74, 0x69, 0x66, 0x79, 0x74, 0x01, 0x12, 0x63, 0x6f, 0x6e, 0x6e, 0x65, 0x63, 0x74, 0x69, 0x6f, 0x6e, 0x2e, 0x62, 0x6c, 0x6f, 0x63, 0x6b, 0x65, 0x64, 0x74, 0x01, 0x13, 0x63, 0x6f, 0x6e, 0x73, 0x75, 0x6d, 0x65, 0x72, 0x5f, 0x70, 0x72, 0x69, 0x6f, 0x72, 0x69, 0x74, 0x69, 0x65, 0x73, 0x74, 0x01, 0x1c, 0x61, 0x75, 0x74, 0x68, 0x65, 0x6e, 0x74, 0x69, 0x63, 0x61, 0x74, 0x69, 0x6f, 0x6e, 0x5f, 0x66, 0x61, 0x69, 0x6c, 0x75, 0x72, 0x65, 0x5f, 0x63, 0x6c, 0x6f, 0x73, 0x65, 0x74, 0x01, 0x10, 0x70, 0x65, 0x72, 0x5f, 0x63, 0x6f, 0x6e, 0x73, 0x75, 0x6d, 0x65, 0x72, 0x5f, 0x71, 0x6f, 0x73, 0x74, 0x01, 0x0f, 0x64, 0x69, 0x72, 0x65, 0x63, 0x74, 0x5f, 0x72, 0x65, 0x70, 0x6c, 0x79, 0x5f, 0x74, 0x6f, 0x74, 0x01, 0x0c, 0x63, 0x6c, 0x75, 0x73, 0x74, 0x65, 0x72, 0x5f, 0x6e, 0x61, 0x6d, 0x65, 0x53, 0x00, 0x00, 0x00, 0x12, 0x72, 0x61, 0x62, 0x62, 0x69, 0x74, 0x40, 0x6e, 0x61, 0x74, 0x68, 0x61, 0x6e, 0x73, 0x2d, 0x6d, 0x62, 0x70, 0x09, 0x63, 0x6f, 0x70, 0x79, 0x72, 0x69, 0x67, 0x68, 0x74, 0x53, 0x00, 0x00, 0x00, 0x37, 0x43, 0x6f, 0x70, 0x79, 0x72, 0x69, 0x67, 0x68, 0x74, 0x20, 0x28, 0x63, 0x29, 0x20, 0x32, 0x30, 0x30, 0x37, 0x2d, 0x32, 0x30, 0x32, 0x33, 0x20, 0x56, 0x4d, 0x77, 0x61, 0x72, 0x65, 0x2c, 0x20, 0x49, 0x6e, 0x63, 0x2e, 0x20, 0x6f, 0x72, 0x20, 0x69, 0x74, 0x73, 0x20, 0x61, 0x66, 0x66, 0x69, 0x6c, 0x69, 0x61, 0x74, 0x65, 0x73, 0x2e, 0x0b, 0x69, 0x6e, 0x66, 0x6f, 0x72, 0x6d, 0x61, 0x74, 0x69, 0x6f, 0x6e, 0x53, 0x00, 0x00, 0x00, 0x39, 0x4c, 0x69, 0x63, 0x65, 0x6e, 0x73, 0x65, 0x64, 0x20, 0x75, 0x6e, 0x64, 0x65, 0x72, 0x20, 0x74, 0x68, 0x65, 0x20, 0x4d, 0x50, 0x4c, 0x20, 0x32, 0x2e, 0x30, 0x2e, 0x20, 0x57, 0x65, 0x62, 0x73, 0x69, 0x74, 0x65, 0x3a, 0x20, 0x68, 0x74, 0x74, 0x70, 0x73, 0x3a, 0x2f, 0x2f, 0x72, 0x61, 0x62, 0x62, 0x69, 0x74, 0x6d, 0x71, 0x2e, 0x63, 0x6f, 0x6d, 0x08, 0x70, 0x6c, 0x61, 0x74, 0x66, 0x6f, 0x72, 0x6d, 0x53, 0x00, 0x00, 0x00, 0x11, 0x45, 0x72, 0x6c, 0x61, 0x6e, 0x67, 0x2f, 0x4f, 0x54, 0x50, 0x20, 0x32, 0x36, 0x2e, 0x30, 0x2e, 0x32, 0x07, 0x70, 0x72, 0x6f, 0x64, 0x75, 0x63, 0x74, 0x53, 0x00, 0x00, 0x00, 0x08, 0x52, 0x61, 0x62, 0x62, 0x69, 0x74, 0x4d, 0x51, 0x07, 0x76, 0x65, 0x72, 0x73, 0x69, 0x6f, 0x6e, 0x53, 0x00, 0x00, 0x00, 0x06, 0x33, 0x2e, 0x31, 0x32, 0x2e, 0x34, 0x00, 0x00, 0x00, 0x0e, 0x50, 0x4c, 0x41, 0x49, 0x4e, 0x20, 0x41, 0x4d, 0x51, 0x50, 0x4c, 0x41, 0x49, 0x4e, 0x00, 0x00, 0x00, 0x05, 0x65, 0x6e, 0x5f, 0x55, 0x53, 0xce};

// char con[] = "\x01\x00\x00\x00\x00\x01\xf9\x00\x0a\x00\x0a\x00\x09\x00\x00\x01\xd4\x0c\x63\x61\x70\x61\x62\x69\x6c\x69\x74\x69\x65\x73\x46\x00\x00\x00\xc7\x12\x70\x75\x62\x6c\x69\x73\x68\x65\x72\x5f\x63\x6f\x6e\x66\x69\x72\x6d\x73\x74\x01\x1a\x65\x78\x63\x68\x61\x6e\x67\x65\x5f\x65\x78\x63\x68\x61\x6e\x67\x65\x5f\x62\x69\x6e\x64\x69\x6e\x67\x73\x74\x01\x0a\x62\x61\x73\x69\x63\x2e\x6e\x61\x63\x6b\x74\x01\x16\x63\x6f\x6e\x73\x75\x6d\x65\x72\x5f\x63\x61\x6e\x63\x65\x6c\x5f\x6e\x6f\x74\x69\x66\x79\x74\x01\x12\x63\x6f\x6e\x6e\x65\x63\x74\x69\x6f\x6e\x2e\x62\x6c\x6f\x63\x6b\x65\x64\x74\x01\x13\x63\x6f\x6e\x73\x75\x6d\x65\x72\x5f\x70\x72\x69\x6f\x72\x69\x74\x69\x65\x73\x74\x01\x1c\x61\x75\x74\x68\x65\x6e\x74\x69\x63\x61\x74\x69\x6f\x6e\x5f\x66\x61\x69\x6c\x75\x72\x65\x5f\x63\x6c\x6f\x73\x65\x74\x01\x10\x70\x65\x72\x5f\x63\x6f\x6e\x73\x75\x6d\x65\x72\x5f\x71\x6f\x73\x74\x01\x0f\x64\x69\x72\x65\x63\x74\x5f\x72\x65\x70\x6c\x79\x5f\x74\x6f\x74\x01\x0c\x63\x6c\x75\x73\x74\x65\x72\x5f\x6e\x61\x6d\x65\x53\x00\x00\x00\x12\x72\x61\x62\x62\x69\x74\x40\x6e\x61\x74\x68\x61\x6e\x73\x2d\x6d\x62\x70\x09\x63\x6f\x70\x79\x72\x69\x67\x68\x74\x53\x00\x00\x00\x37\x43\x6f\x70\x79\x72\x69\x67\x68\x74\x20\x28\x63\x29\x20\x32\x30\x30\x37\x2d\x32\x30\x32\x33\x20\x56\x4d\x77\x61\x72\x65\x2c\x20\x49\x6e\x63\x2e\x20\x6f\x72\x20\x69\x74\x73\x20\x61\x66\x66\x69\x6c\x69\x61\x74\x65\x73\x2e\x0b\x69\x6e\x66\x6f\x72\x6d\x61\x74\x69\x6f\x6e\x53\x00\x00\x00\x39\x4c\x69\x63\x65\x6e\x73\x65\x64\x20\x75\x6e\x64\x65\x72\x20\x74\x68\x65\x20\x4d\x50\x4c\x20\x32\x2e\x30\x2e\x20\x57\x65\x62\x73\x69\x74\x65\x3a\x20\x68\x74\x74\x70\x73\x3a\x2f\x2f\x72\x61\x62\x62\x69\x74\x6d\x71\x2e\x63\x6f\x6d\x08\x70\x6c\x61\x74\x66\x6f\x72\x6d\x53\x00\x00\x00\x11\x45\x72\x6c\x61\x6e\x67\x2f\x4f\x54\x50\x20\x32\x36\x2e\x30\x2e\x32\x07\x70\x72\x6f\x64\x75\x63\x74\x53\x00\x00\x00\x08\x52\x61\x62\x62\x69\x74\x4d\x51\x07\x76\x65\x72\x73\x69\x6f\x6e\x53\x00\x00\x00\x06\x33\x2e\x31\x32\x2e\x34\x00\x00\x00\x0e\x50\x4c\x41\x49\x4e\x20\x41\x4d\x51\x50\x4c\x41\x49\x4e\x00\x00\x00\x05\x65\x6e\x5f\x55\x53\xce";
// const char* teste_con = "\x01\x00\x00\x00\x00\x01\xf9\x00\x0a\x00\x0a\x00";

typedef struct amqp_protocol_header {
    char protocol[4];
    u_int8_t protocol_id;
    u_int8_t protocol_v1;
    u_int8_t protocol_v2;
    u_int8_t protocol_v3;
} amqp_protocol_header;

amqp_protocol_header default_protocol_header = {
    .protocol = "AMQP",
    .protocol_id = 0,
    .protocol_v1 = 0,
    .protocol_v2 = 9,
    .protocol_v3 = 1
};

typedef struct amqp_protocol_package {
    u_int8_t type;
    u_int8_t channel;
    u_int16_t payload_size;
    u_int16_t class_id;
    u_int16_t method_id;
    u_int8_t version_major;
    u_int8_t version_minor;
    u_int8_t server_properties;
    u_int8_t mechanisms;
    u_int8_t locales;
} amqp_protocol_package;

int read_protocol_header (ClientThread *client) {
    int header_size = sizeof (default_protocol_header);

    printf ("header_size: %d\n", header_size);

    if ((client->n=read(client->connfd, client->recvline, MAXLINE)) != header_size) {
        perror("protocol header with different size :(\n");
        return 1;
    }
    if (memcmp (client->recvline, &default_protocol_header, header_size) != 0) {
        perror("protocol header with different values :(\n");
        return 1;
    }
    printf ("protocol header accepted!\n");
    return 0;
}

void *handle_client(void *arg) {
    ClientThread *client = (ClientThread *)arg;

    /**** THREAD FILHA ****/
    printf("[Uma conexão aberta]\n");

    read_protocol_header (client);

    // write (client->connfd, connection_start, sizeof(connection_start));

    // if ((client->n=read(client->connfd, client->recvline, MAXLINE)) > 0) {
    //     // temos que verificar se mandou um protocol header bom
    //     client->recvline[client->n]=0;
    //     printf ("Connection-Ok: %s\n", client->recvline);
    // }

    // while ((n=read(client->connfd, recvline, MAXLINE)) > 0) {
    //     recvline[n]=0;
    //     printf("[Cliente conectado na thread filha de id %d enviou:] ", client->connfd);
    //     if ((fputs(recvline,stdout)) == EOF) {
    //         perror("fputs :( \n");
    //         exit(6);
    //     }
    //     write(client->connfd, recvline, strlen(recvline));
    // }
    // printf ("%d\n", queueCount);

    printf("[Uma conexão fechada]\n");

    return NULL;
}

int main (int argc, char **argv) {
    /* Os sockets. Um que será o socket que vai escutar pelas conexões
     * e o outro que vai ser o socket específico de cada conexão
     */
    int listenfd, connfd;
    /* Informações sobre o socket (endereço e porta) ficam nesta struct
     */
    struct sockaddr_in servaddr;
   
    if (argc != 2) {
        fprintf(stderr,"Uso: %s <Porta>\n",argv[0]);
        fprintf(stderr,"Vai rodar um servidor de echo na porta <Porta> TCP\n");
        exit(1);
    }

    /* Criação de um socket. É como se fosse um descritor de arquivo.
     * É possível fazer operações como read, write e close. Neste caso o
     * socket criado é um socket IPv4 (por causa do AF_INET), que vai
     * usar TCP (por causa do SOCK_STREAM), já que o AMQP funciona sobre
     * TCP, e será usado para uma aplicação convencional sobre a Internet
     * (por causa do número 0)
     */
    if ((listenfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("socket :(\n");
        exit(2);
    }

    /* Agora é necessário informar os endereços associados a este
     * socket. É necessário informar o endereço / interface e a porta,
     * pois mais adiante o socket ficará esperando conexões nesta porta
     * e neste(s) endereços. Para isso é necessário preencher a struct
     * servaddr. É necessário colocar lá o tipo de socket (No nosso
     * caso AF_INET porque é IPv4), em qual endereço / interface serão
     * esperadas conexões (Neste caso em qualquer uma -- INADDR_ANY) e
     * qual a porta. Neste caso será a porta que foi passada como
     * argumento no shell (atoi(argv[1])). No caso do servidor AMQP,
     * utilize a porta padrão do protocolo que você deve descobrir
     * lendo a especificaçao dele ou capturando os pacotes de conexões
     * ao RabbitMQ com o Wireshark
     */
    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family      = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port        = htons(atoi(argv[1]));
    if (bind(listenfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) == -1) {
        perror("bind :(\n");
        exit(3);
    }

    /* Como este código é o código de um servidor, o socket será um
     * socket que escutará por conexões. Para isto é necessário chamar
     * a função listen que define que este é um socket de servidor que
     * ficará esperando por conexões nos endereços definidos na função
     * bind.
     */
    if (listen(listenfd, LISTENQ) == -1) {
        perror("listen :(\n");
        exit(4);
    }

    printf("[Servidor no ar. Aguardando conexões na porta %s]\n",argv[1]);
    printf("[Para finalizar, pressione CTRL+c ou rode um kill ou killall]\n");
   
    /* O servidor no final das contas é um loop infinito de espera por
     * conexões e processamento de cada uma individualmente
     */
	for (int cur_thread = 0; ; cur_thread++) {
        if (cur_thread >= MAX_THREADS) {
            perror("Maximum number of threads reached :(\n");
            exit(7);
        }
        /* O socket inicial que foi criado é o socket que vai aguardar
         * pela conexão na porta especificada. Mas pode ser que existam
         * diversos clientes conectando no servidor. Por isso deve-se
         * utilizar a função accept. Esta função vai retirar uma conexão
         * da fila de conexões que foram aceitas no socket listenfd e
         * vai criar um socket específico para esta conexão. O descritor
         * deste novo socket é o retorno da função accept.
         */
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
