#include "packages.h"

int read_protocol_header (ClientThread *client) {
    size_t header_size = sizeof (default_protocol_header);

    printf ("header_size: %zu\n", header_size);

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

    printf ("frame header: %02x %02x %02x %02x %02x %02x %02x\n", client->recvline[0], client->recvline[1], client->recvline[2], client->recvline[3], client->recvline[4], client->recvline[5], client->recvline[6]);

    int payload_size = find_payload_size (client);
    printf ("payload_size: %d\n", payload_size);

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
    if (read_protocol_header (client) != 0) {
        printf("[Uma conexão fechada por header incorreto]\n");
        return 1;
    }

    send_connection_start (client);

    if (read_connection_start_ok (client) != 0) {
        printf("[Uma conexão fechada por connection_start_ok incorreto]\n");
        return 1;
    }

    send_connection_tune (client);

    if (read_connection_tune_ok (client) != 0) {
        printf("[Uma conexão fechada por connection_tune_ok incorreto]\n");
        return 1;
    }

    if (read_connection_open (client) != 0) {
        printf("[Uma conexão fechada por connection_open incorreto]\n");
        return 1;
    }

    send_connection_open_ok (client);

    if (read_channel_open (client) != 0) {
        printf("[Uma conexão fechada por channel_open incorreto]\n");
        return 1;
    }

    send_channel_open_ok (client);
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