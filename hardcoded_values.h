#include <stdio.h>

#define TYPE_METHOD 1

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

#define CLASS_QUEUE 50
#define METHOD_DECLARE 10
#define METHOD_DECLARE_OK 11

extern u_int8_t connection_start_brute [];
extern u_int8_t connection_tune_brute [];
extern u_int8_t connection_open_ok_brute [];
extern u_int8_t channel_open_ok_brute [];
extern u_int8_t channel_close_ok_brute [];
extern u_int8_t connection_close_ok_brute [];

extern size_t connection_start_brute_size;
extern size_t connection_tune_brute_size;
extern size_t connection_open_ok_brute_size;
extern size_t channel_open_ok_brute_size;
extern size_t channel_close_ok_brute_size;
extern size_t connection_close_ok_brute_size;

typedef struct amqp_protocol_header {
    char protocol[4];
    u_int8_t protocol_id;
    u_int8_t protocol_v1;
    u_int8_t protocol_v2;
    u_int8_t protocol_v3;
} amqp_protocol_header;

extern amqp_protocol_header default_protocol_header;

typedef struct amqp_protocol_package {
    u_int8_t type;
    u_int16_t channel;
    u_int32_t payload_size;
    u_int16_t class_id;
    u_int16_t method_id;
} __attribute__((packed)) amqp_protocol_package;

void unparse_package (amqp_protocol_package *package);
void parse_package (amqp_protocol_package *package);
