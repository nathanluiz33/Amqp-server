#ifndef HARDCODED_VALUES_H
#define HARDCODED_VALUES_H

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

#endif