#ifndef COMMON_H
#define COMMON_H

#include "cuckoo_hash.h"

#define buf_size 1024
#define max_num_client 1024
#define MAX_CLIENT max_num_client

#define calloc_struct(x) calloc(sizeof(x),1)

struct client_info
{
  int client_fd;
  struct cuckoo_hash* hash_table;
};

struct server_cb
{
  int server_sock;
  struct client_info** client_fd_list;
};

struct request_hdr
{
  int function;
  int key_length;
  int value_length;
};

#define req_hdr_size sizeof(struct request_hdr)

struct response_hdr
{
  int content_length;
  int error_code;
};

#define res_hdr_size sizeof(struct response_hdr)

struct result
{
  int err;
  char* payload;
};

/* error codes */
#define OK 200
#define ERR_NOT_FOUND 404
#define ERR_BAD_REQUEST 400
#define ERR_LENGTH_REQUIRED 411
#define ERR_UNIMPLEMENTED 501

#define ERR_INTERNAL 500

/* function op code */
#define INIT_HASH_TABLE 0
#define PUT 1
#define GET 2


#define perror(x) fprintf(stderr, x)





#endif
