#include "packages.h"

amqp_protocol_header default_protocol_header = {
    .protocol = "AMQP",
    .protocol_id = 0,
    .protocol_v1 = 0,
    .protocol_v2 = 9,
    .protocol_v3 = 1
};

void unparse_general_frame_header (general_frame_header *package) {
    package->channel = ntohs (package->channel);
    package->payload_size = ntohl (package->payload_size);
}

void parse_general_frame_header (general_frame_header *package) {
    package->channel = htons (package->channel);
    package->payload_size = htonl (package->payload_size);
}

void unparse_method_payloads_header (method_payloads_header *package) {
    package->method_id = ntohs (package->method_id);
    package->class_id = ntohs (package->class_id);
}

void parse_method_payloads_header (method_payloads_header *package) {
    package->method_id = htons (package->method_id);
    package->class_id = htons (package->class_id);
}

void unparse_content_header (content_header *package) {
    package->class_id = ntohs (package->class_id);
    package->weight = ntohs (package->weight);
    package->body_size = ntohll (package->body_size);
}

void parse_content_header (content_header *package) {
    package->class_id = htons (package->class_id);
    package->weight = htons (package->weight);
    package->body_size = htonll (package->body_size);
}


int read_protocol_header (ClientThread *client) {
    size_t header_size = sizeof (default_protocol_header);

    if ((client->n=read(client->connfd, client->recvline, MAXLINE)) < header_size) {
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

int find_payload_size (ClientThread *client) {
    int payload_size = (1 << 24) * client->recvline[3] + (1 << 16) * client->recvline[4] + (1 << 8) * client->recvline[5] + client->recvline[6];
    return payload_size;
}

int read_next_method (ClientThread *client) {
    {
        size_t size = read(client->connfd, client->recvline, FRAME_HEADER_SIZE);
        if (size < FRAME_HEADER_SIZE) return 1;
    }
    int payload_size = find_payload_size (client);
    {
        size_t size = read(client->connfd, client->recvline + FRAME_HEADER_SIZE, payload_size + 1);
        if (size < payload_size + 1) return 1;
    }

    return 0;
}

void send_method_brute (ClientThread *client, u_int8_t hardcoded[], size_t size) {
    memcpy (client->sendline, hardcoded, size);
    client->sendline[size] = '\xce';
    write (client->connfd, client->sendline, size + 1);
}

void send_connection_start (ClientThread *client) {
    send_method_brute (client, connection_start_brute, connection_start_brute_size);
}

int read_connection_start_ok (ClientThread *client) {
    if (read_next_method (client) != 0) return 1;

    if (client->recvline[0] != 1 || (16 * client->recvline[7] + client->recvline[8] != CLASS_CONNECTION) || (16 * client->recvline[9] + client->recvline[10] != METHOD_CONNECTION_START_OK)) return 1;

    printf ("connection start ok accepted!\n");
    return 0;
}

void send_connection_tune (ClientThread *client) {
    send_method_brute (client, connection_tune_brute, connection_tune_brute_size);
}

int read_connection_tune_ok (ClientThread *client) {
    if (read_next_method (client) != 0) return 1;

    if (client->recvline[0] != 1 || (16 * client->recvline[7] + client->recvline[8] != CLASS_CONNECTION) || (16 * client->recvline[9] + client->recvline[10] != METHOD_CONNECTION_TUNE_OK)) return 1;

    printf ("connection tune ok accepted!\n");
    return 0;
}

int read_connection_open (ClientThread *client) {
    if (read_next_method (client) != 0) return 1;
    if (client->recvline[0] != 1 || (16 * client->recvline[7] + client->recvline[8] != CLASS_CONNECTION) || (16 * client->recvline[9] + client->recvline[10] != METHOD_CONNECTION_OPEN)) return 1;

    printf ("connection open accepted!\n");
    return 0;
}

void send_connection_open_ok (ClientThread *client) {
    send_method_brute (client, connection_open_ok_brute, connection_open_ok_brute_size);
}

int read_channel_open (ClientThread *client) {
    if (read_next_method (client) != 0) return 1;
    if (client->recvline[0] != 1 || (16 * client->recvline[7] + client->recvline[8] != CLASS_CHANNEL) || (16 * client->recvline[9] + client->recvline[10] != METHOD_CHANNEL_OPEN)) return 1;

    printf ("channel open accepted!\n");
    return 0;
}

void send_channel_open_ok (ClientThread *client) {
    send_method_brute (client, channel_open_ok_brute, channel_open_ok_brute_size);
}

int initialize_connection (ClientThread *client) {
    pthread_mutex_lock(&client->clientMutex);
    if (read_protocol_header (client) != 0) {
        pthread_mutex_unlock(&client->clientMutex);
        printf("[Uma conexão fechada por header incorreto]\n");
        return 1;
    }
    pthread_mutex_unlock(&client->clientMutex);


    pthread_mutex_lock(&client->clientMutex);
    send_connection_start (client);
    pthread_mutex_unlock(&client->clientMutex);

    pthread_mutex_lock(&client->clientMutex);
    if (read_connection_start_ok (client) != 0) {
        pthread_mutex_unlock(&client->clientMutex);
        printf("[Uma conexão fechada por connection_start_ok incorreto]\n");
        return 1;
    }
    pthread_mutex_unlock(&client->clientMutex);

    pthread_mutex_lock(&client->clientMutex);
    send_connection_tune (client);
    pthread_mutex_unlock(&client->clientMutex);

    pthread_mutex_lock(&client->clientMutex);
    if (read_connection_tune_ok (client) != 0) {
        pthread_mutex_unlock(&client->clientMutex);
        printf("[Uma conexão fechada por connection_tune_ok incorreto]\n");
        return 1;
    }
    pthread_mutex_unlock(&client->clientMutex);

    pthread_mutex_lock(&client->clientMutex);
    if (read_connection_open (client) != 0) {
        pthread_mutex_unlock(&client->clientMutex);
        printf("[Uma conexão fechada por connection_open incorreto]\n");
        return 1;
    }
    pthread_mutex_unlock(&client->clientMutex);

    pthread_mutex_lock(&client->clientMutex);
    send_connection_open_ok (client);
    pthread_mutex_unlock(&client->clientMutex);

    pthread_mutex_lock(&client->clientMutex);
    if (read_channel_open (client) != 0) {
        pthread_mutex_unlock(&client->clientMutex);
        printf("[Uma conexão fechada por channel_open incorreto]\n");
        return 1;
    }
    pthread_mutex_unlock(&client->clientMutex);

    pthread_mutex_lock(&client->clientMutex);
    send_channel_open_ok (client);
    pthread_mutex_unlock(&client->clientMutex);
    return 0;
}

int read_channel_close (ClientThread *client) {
    if (read_next_method (client) != 0) return 1;
    if (client->recvline[0] != 1 || (16 * client->recvline[7] + client->recvline[8] != CLASS_CHANNEL) || (16 * client->recvline[9] + client->recvline[10] != METHOD_CHANNEL_CLOSE)) return 1;

    printf ("channel close accepted!\n");
    return 0;
}

int read_connection_close (ClientThread *client) {
    if (read_next_method (client) != 0) return 1;
    if (client->recvline[0] != 1 || (16 * client->recvline[7] + client->recvline[8] != CLASS_CONNECTION) || (16 * client->recvline[9] + client->recvline[10] != METHOD_CONNECTION_CLOSE)) return 1;

    printf ("connection close accepted!\n");
    return 0;
}

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

    read_next_method (client);

    content_header* content = (content_header*)malloc(sizeof(content_header));
    memcpy (content, client->recvline + sizeof (general_frame_header), sizeof (*content));

    unparse_content_header (content);

    read_next_method (client);

    char *body = (char*)malloc(content->body_size + 1);
    memcpy (body, client->recvline + sizeof (general_frame_header), content->body_size);

    publish_AmqpQueue (queue_name, body);
}

void send_declare_queue_ok (ClientThread *client, char *queue_name) {
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

void handle_deliver (ClientThread *client, const char queue_name[], const char data[]) {
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
        __content_header->body_size = strlen (data);

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
        general_frame->payload_size = strlen (data);
        unparse_general_frame_header (general_frame);

        int cur = 0;
        memcpy (client->sendline + cur, general_frame, sizeof (general_frame_header));
        cur += sizeof (general_frame_header);
        memcpy (client->sendline + cur, data, strlen (data));
        cur += strlen (data);

        client->sendline[cur] = '\xce';
        write (client->connfd, client->sendline, cur + 1);
    }
}

void handle_consume (ClientThread *client) {
    char queue_name[256];

    int size = client->recvline[14];
    memcpy (queue_name, client->recvline + 14, size);
    queue_name[size] = '\0';

    pthread_mutex_unlock(&client->clientMutex);

    printf ("AQYUUUUUUIIIIII\n\n\n");
    if (add_client_to_AmqpQueue (queue_name, client)) {
        printf ("Fila não existente\n");
        // devemos mandar mensagem de erro
        send_method_brute (client, queue_not_found_brute, queue_not_found_brute_size);
        return;
    }
    printf ("DESBLOQUEANDO\n\n\n");

    while (1) {}
}

int discover_which_method (ClientThread *client) {
    pthread_mutex_lock(&client->clientMutex);
    if (read_next_method (client)) {
        pthread_mutex_unlock(&client->clientMutex);
        return 1;
    }
    pthread_mutex_unlock(&client->clientMutex);

    general_frame_header* general_frame = (general_frame_header*)malloc(sizeof(general_frame_header));
    method_payloads_header* method_payloads = (method_payloads_header*)malloc(sizeof(method_payloads_header));

    size_t shift = sizeof (general_frame_header);
    memcpy (general_frame, client->recvline, sizeof (*general_frame));
    memcpy (method_payloads, client->recvline + shift, sizeof (*method_payloads));

    unparse_general_frame_header (general_frame);
    unparse_method_payloads_header (method_payloads);
    
    if (general_frame->type != TYPE_METHOD || general_frame->channel != 1) return 1;
    
    pthread_mutex_lock(&client->clientMutex);
    if (method_payloads->class_id == CLASS_BASIC && method_payloads->method_id == METHOD_PUBLISH) {
        handle_publish (client);
        pthread_mutex_unlock(&client->clientMutex);
        return 0;
    }
    else if (method_payloads->class_id == CLASS_BASIC && method_payloads->method_id == METHOD_CONSUME) {
        handle_consume (client);
        return 2;
    }
    else if (method_payloads->class_id == CLASS_QUEUE && method_payloads->method_id == METHOD_DECLARE) {
        handle_queue_declare (client);
        pthread_mutex_unlock(&client->clientMutex);
        return 0;
    }
    else {
        printf ("Metodo nao suportado pelo servidor\n");
        pthread_mutex_unlock(&client->clientMutex);
        return 1;
    }
}

int finish_connection (ClientThread *client) {
    pthread_mutex_lock(&client->clientMutex);
    if (read_channel_close (client) != 0) {
        pthread_mutex_unlock(&client->clientMutex);
        printf("[Uma conexão fechada por channel_close incorreto]\n");
        return 1;
    }
    pthread_mutex_unlock(&client->clientMutex);

    pthread_mutex_lock(&client->clientMutex);
    send_method_brute (client, channel_close_ok_brute, channel_close_ok_brute_size);
    pthread_mutex_unlock(&client->clientMutex);

    pthread_mutex_lock(&client->clientMutex);
    if (read_connection_close (client) != 0) {
        pthread_mutex_unlock(&client->clientMutex);
        printf("[Uma conexão fechada por connection_close incorreto]\n");
        return 1;
    }
    pthread_mutex_unlock(&client->clientMutex);

    pthread_mutex_lock(&client->clientMutex);
    send_method_brute (client, connection_close_ok_brute, connection_close_ok_brute_size);
    pthread_mutex_unlock(&client->clientMutex);
    return 0;
}