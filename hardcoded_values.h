#include <stdio.h>

#define TYPE_METHOD 1
#define TYPE_CONTENT_HEADER 2
#define TYPE_CONTENT_BODY 3

#define CLASS_CONNECTION 10
#define METHOD_CONNECTION_START_OK 11
#define METHOD_CONNECTION_TUNE_OK 31
#define METHOD_CONNECTION_OPEN 40
#define METHOD_CONNECTION_CLOSE 50

#define CLASS_CHANNEL 20
#define METHOD_CHANNEL_OPEN 10
#define METHOD_CHANNEL_CLOSE 40

#define CLASS_BASIC 60
#define METHOD_PUBLISH 40
#define METHOD_CONSUME 20
#define METHOD_CONSUME_OK 21
#define METHOD_DELIVER 60
#define METHOD_ACK 80

#define CLASS_QUEUE 50
#define METHOD_DECLARE 10
#define METHOD_DECLARE_OK 11

extern u_int8_t connection_start_brute [];
extern u_int8_t connection_tune_brute [];
extern u_int8_t connection_open_ok_brute [];
extern u_int8_t channel_open_ok_brute [];
extern u_int8_t channel_close_ok_brute [];
extern u_int8_t connection_close_ok_brute [];
extern u_int8_t deliver_method_args_brute [];
extern u_int8_t method_consume_ok_brute [];
extern u_int8_t queue_not_found_brute [];

extern size_t connection_start_brute_size;
extern size_t connection_tune_brute_size;
extern size_t connection_open_ok_brute_size;
extern size_t channel_open_ok_brute_size;
extern size_t channel_close_ok_brute_size;
extern size_t connection_close_ok_brute_size;
extern size_t deliver_method_args_brute_size;
extern size_t method_consume_ok_brute_size;
extern size_t queue_not_found_brute_size;

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
