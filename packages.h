#ifndef PACKAGES_H
#define PACKAGES_H

#include <stdio.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <unistd.h>
#include "hardcoded_values.h"
#include "amqp_queues.h"
#include "client_thread.h"
#include <pthread.h>

#define FRAME_HEADER_SIZE 7
#define MAXLINE 4096

typedef struct amqp_protocol_header {
    char protocol[4];
    u_int8_t protocol_id;
    u_int8_t protocol_v1;
    u_int8_t protocol_v2;
    u_int8_t protocol_v3;
} amqp_protocol_header;

extern amqp_protocol_header default_protocol_header;

typedef struct general_frame_header {
    u_int8_t type;
    u_int16_t channel;
    u_int32_t payload_size;
} __attribute__((packed)) general_frame_header;

typedef struct method_payloads_header {
    u_int16_t class_id;
    u_int16_t method_id;
} __attribute__((packed)) method_payloads_header;

typedef struct content_header {
    u_int16_t class_id;
    u_int16_t weight;
    u_int64_t body_size;
} __attribute__((packed)) content_header;


void unparse_general_frame_header (general_frame_header *package);
void parse_general_frame_header (general_frame_header *package);

void unparse_method_payloads_header (method_payloads_header *package);
void parse_method_payloads_header (method_payloads_header *package);

void unparse_content_header (content_header *package);
void parse_content_header (content_header *package);

// START CONNECTION
int initialize_connection (ClientThread *client);

// READ METHODS
int read_next_method (ClientThread *client);
void send_method_brute (ClientThread *client, u_int8_t hardcoded[], size_t size);

// FINISH CONNECTION
int finish_connection (ClientThread *client);


// METHODS
void handle_publish (ClientThread *client);

void send_declare_queue_ok (ClientThread *client, char *queue_name);

void handle_queue_declare (ClientThread *client);

void handle_deliver (ClientThread *client, const char queue_name[], const char data[]);

void handle_consume (ClientThread *client);

int discover_which_method (ClientThread *client);

#endif