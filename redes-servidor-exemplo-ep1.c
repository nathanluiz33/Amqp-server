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

void handle_publish (ClientThread *client) {
    char queue_name[256];

    int size = -1;
    {
        int cur = 13;
        cur += client->recvline[cur] + 1;

        size = client->recvline[cur];
        // printf ("size: %d\n", size);
        memcpy (queue_name, client->recvline + cur + 1, size);
        queue_name[size] = '\0';
    }

    // printf ("PUBLISH queue_name: %s\n", queue_name);

    read_next_method (client);

    content_header* content = (content_header*)malloc(sizeof(content_header));
    memcpy (content, client->recvline + sizeof (general_frame_header), sizeof (*content));

    unparse_content_header (content);

    // printf ("content: %d %d %llu\n", content->class_id, content->weight, content->body_size);

    read_next_method (client);

    char *body = (char*)malloc(content->body_size + 1);
    memcpy (body, client->recvline + sizeof (general_frame_header), content->body_size);

    // printf ("body: %s\n", body);

    publish_AmqpQueue (queue_name, body);
}

void send_declare_queue_ok (ClientThread *client, char *queue_name) {
    // printf ("aquiqueue_name: %s\n", queue_name);

    general_frame_header* general_frame = (general_frame_header*)malloc(sizeof(general_frame_header));
    method_payloads_header* method_payloads = (method_payloads_header*)malloc(sizeof(method_payloads_header));

    general_frame->type = TYPE_METHOD;
    general_frame->channel = 1;
    method_payloads->class_id = CLASS_QUEUE;
    method_payloads->method_id = METHOD_DECLARE_OK;

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

    general_frame->payload_size = cur - 7;

    unparse_general_frame_header (general_frame);
    unparse_method_payloads_header (method_payloads);

    memcpy (client->sendline, general_frame, sizeof (*general_frame));
    memcpy (client->sendline + sizeof (*general_frame), method_payloads, sizeof (*method_payloads));

    client->sendline[cur] = '\xce';

    write (client->connfd, client->sendline, cur + 1);

    printf ("declare queue ok sent!\n");
}

void handle_queue_declare (ClientThread *client) {
    char queue_name[256];

    int size = client->recvline[14];
    memcpy (queue_name, client->recvline + 14, size);
    queue_name[size] = '\0';

    declare_AmqpQueue (queue_name);

    send_declare_queue_ok (client, queue_name);
}

void handle_deliver (ClientThread *client, const char queue_name[]) {
    printf ("TOOOAQUIIIIIIIIIIIIIIII\n\n");
    send_method_brute (client, method_consume_ok_brute, method_consume_ok_brute_size);
    {
        general_frame_header* general_frame = (general_frame_header*)malloc(sizeof(general_frame_header));
        method_payloads_header* method_payloads = (method_payloads_header*)malloc(sizeof(method_payloads_header));
        
        general_frame->type = TYPE_METHOD;
        general_frame->channel = 1;
        method_payloads->class_id = CLASS_BASIC;
        method_payloads->method_id = METHOD_DELIVER;

        u_int32_t payload_size = (u_int32_t) sizeof (method_payloads_header) + deliver_method_args_brute_size + 1 + strlen (queue_name);
        general_frame->payload_size = payload_size;

        int cur = sizeof (general_frame_header) + sizeof (method_payloads_header);
        memcpy (client->sendline + cur, deliver_method_args_brute, deliver_method_args_brute_size);
        cur += deliver_method_args_brute_size;

        client->sendline[cur] = strlen (queue_name);
        memcpy (client->sendline + cur + 1, queue_name, strlen (queue_name));
        cur += strlen (queue_name) + 1;



        unparse_general_frame_header (general_frame);
        unparse_method_payloads_header (method_payloads);
        printf ("payload_size: %d\n", payload_size);

        memcpy (client->sendline, general_frame, sizeof (general_frame_header));
        memcpy (client->sendline + sizeof (general_frame_header), method_payloads, sizeof (*method_payloads));

        client->sendline[cur] = '\xce';

        write (client->connfd, client->sendline, cur + 1);
    }
    {
        general_frame_header* general_frame = (general_frame_header*)malloc(sizeof(general_frame_header));
        content_header* __content_header = (content_header*)malloc(sizeof(content_header));
        
        general_frame->type = TYPE_CONTENT_HEADER;
        general_frame->channel = 1;
        general_frame->payload_size = 15;
        __content_header->class_id = CLASS_BASIC;
        __content_header->weight = 0;
        __content_header->body_size = strlen (client->client_output_queue->front->data);

        unparse_general_frame_header (general_frame);
        unparse_content_header (__content_header);

        int cur = 0;
        memcpy (client->sendline + cur, general_frame, sizeof (general_frame_header));
        cur += sizeof (general_frame_header);

        memcpy (client->sendline + cur, __content_header, sizeof (*__content_header));
        cur += sizeof (*__content_header);
        
        u_int16_t property_flags = 4096;
        property_flags = htons (property_flags);
        memcpy (client->sendline + cur, &property_flags, 2);
        cur += 2;

        client->sendline[cur] = 1;
        cur++;

        client->sendline[cur] = '\xce';
        
        write (client->connfd, client->sendline, cur + 1);
    }
    {
        general_frame_header* general_frame = (general_frame_header*)malloc(sizeof(general_frame_header));

        general_frame->type = TYPE_CONTENT_BODY;
        general_frame->channel = 1;
        general_frame->payload_size = strlen (client->client_output_queue->front->data);
        unparse_general_frame_header (general_frame);

        int cur = 0;
        memcpy (client->sendline + cur, general_frame, sizeof (general_frame_header));
        cur += sizeof (general_frame_header);
        memcpy (client->sendline + cur, client->client_output_queue->front->data, strlen (client->client_output_queue->front->data));
        cur += strlen (client->client_output_queue->front->data);

        client->sendline[cur] = '\xce';
        write (client->connfd, client->sendline, cur + 1);
    }
}

void handle_consume (ClientThread *client) {
    char queue_name[256];

    int size = client->recvline[14];
    memcpy (queue_name, client->recvline + 14, size);
    queue_name[size] = '\0';

    printf ("CONSUME queue_name: %s\n", queue_name);

    if (add_client_to_AmqpQueue (queue_name, client->connfd, client->client_output_queue)) {
        printf ("Fila não existente\n");
        // devemos mandar mensagem de erro
        send_method_brute (client, queue_not_found_brute, queue_not_found_brute_size);
        return;
    }

    while (1) {
        if (client->client_output_queue->front != NULL) {
            ClientOutput* current = client->client_output_queue->front;

            printf("client_output_data: %s\n", current->data);
            // memcpy (client->sendline, current->data, strlen (current->data));
            handle_deliver (client, queue_name);
            // se o negocio morreu, nao podemos fazer nada
            if (read_next_method (client)) break;

            // printf ("type: %d\n", client->recvline[0]);
            method_payloads_header* method_payloads = (method_payloads_header*)malloc(sizeof(method_payloads_header));
            memcpy (method_payloads, client->recvline + sizeof (general_frame_header), sizeof (*method_payloads));
            unparse_method_payloads_header (method_payloads);
            if (method_payloads->class_id != CLASS_BASIC || method_payloads->method_id != METHOD_ACK) printf ("ACK invalido\n");

            // write (client->connfd, current->data, strlen (current->data));

            client->client_output_queue->front = client->client_output_queue->front->next;
            
            free(current);
        }
    }

    rm_client_from_AmqpQueue (queue_name, client->connfd);
}

int discover_which_method (ClientThread *client) {
    if (read_next_method (client)) return 1;

    general_frame_header* general_frame = (general_frame_header*)malloc(sizeof(general_frame_header));
    method_payloads_header* method_payloads = (method_payloads_header*)malloc(sizeof(method_payloads_header));

    size_t shift = sizeof (general_frame_header);
    memcpy (general_frame, client->recvline, sizeof (*general_frame));
    memcpy (method_payloads, client->recvline + shift, sizeof (*method_payloads));

    unparse_general_frame_header (general_frame);
    unparse_method_payloads_header (method_payloads);
    
    if (general_frame->type != TYPE_METHOD || general_frame->channel != 1) return 1;
    
    if (method_payloads->class_id == CLASS_BASIC && method_payloads->method_id == METHOD_PUBLISH) {
        // printf ("package: %d %d %d %d %d\n", package->type, package->channel, package->payload_size, package->class_id, package->method_id);
        handle_publish (client);
    }
    else if (method_payloads->class_id == CLASS_BASIC && method_payloads->method_id == METHOD_CONSUME) {
        handle_consume (client);
        return 2;
    }
    else if (method_payloads->class_id == CLASS_QUEUE && method_payloads->method_id == METHOD_DECLARE) {
        handle_queue_declare (client);
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
    
    int _consume = discover_which_method (client);
    if (_consume == 1) {
        printf("[Uma conexão fechada por falha na descoberta do método básico]\n");
        return NULL;
    }

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
