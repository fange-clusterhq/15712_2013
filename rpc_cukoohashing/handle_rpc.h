#ifndef handle_rpc_h
#define handle_rpc_h

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <time.h>

/*
 * @brief This function conbine the old bytes with the newly received bytes
 * ready for the processing work.
 * @param received_bytes bytes just recv()'ed
 * @param number_bytes_received The number of bytes received.
 * @param send_request_queue The send request queue.
 * @param client The client who sent the bytes.
 * @return nothing
 */

void handle_receive(char* received_bytes, int size,
    struct server_cb* server_cb,
    int client);

#endif




