#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>

#include <assert.h>

#include "cuckoo_hash.h"
#include "common.h"

struct cuckoo_hash* init_hashtable()
{
  struct cuckoo_hash* hash = calloc_struct(struct cuckoo_hash);
  cuckoo_hash_init(hash, 8);
  return hash;
}

void put(struct cuckoo_hash* table, char* key, char* value)
{
  assert(table != NULL);
  cuckoo_hash_insert(table, (void*)(key), strlen(key), value);
  return;
}

int get(struct cuckoo_hash* table, char* key, char** result)
{
  assert(table != NULL);
  struct cuckoo_hash_item* item = cuckoo_hash_lookup(table, (void*)(key), strlen(key));
  if (item == NULL)
  {
    printf("item not found\n");
    return 0;
  }
  *result = (char*)item->value;
  return strlen((char*)(item->value));
}

void send_msg(char* msg, int size, int client_fd,
    struct server_cb* server_cb)
{
  size_t byte_sent =
    send(client_fd, msg, size, 0);
  if (byte_sent <= 0)
  {
    fprintf(stderr, "send failed");
    server_cb->client_fd_list[client_fd] = 0;
    close(client_fd);
  }
}

char* make_packet(int err, int length, char* payload)
{
  struct response_hdr header;
  header.content_length = length;
  header.error_code = err;
  char* packet = malloc(res_hdr_size + length);
  memcpy(packet, &header, res_hdr_size);
  if (length > 0)
  {
    memcpy(packet + res_hdr_size, payload, length);
  }
  return packet;
}

void handle_receive(char* received_bytes, int size,
    struct server_cb* server_cb,
    int client)
{
  if (size < req_hdr_size)
  {
    char* err_pack = make_packet(ERR_BAD_REQUEST, 0, NULL);
    send_msg(err_pack, res_hdr_size, client, server_cb);
    free(err_pack);
    goto done;
  }
  struct request_hdr header;
  memset(&header, 0, req_hdr_size);
  memcpy(&header, received_bytes, req_hdr_size);
  int key_len = header.key_length;
  int len = header.value_length;
  switch(header.function)
  {
    case INIT_HASH_TABLE:
      server_cb->client_fd_list[client]->hash_table = init_hashtable();
      goto REPLY_OK;
    case PUT:
      if (key_len > 0 && len > 0)
      {
        char* key = malloc(key_len+1);
        memcpy(key, received_bytes+req_hdr_size, key_len);
        char* value = malloc(len+1);
        memcpy(value, received_bytes+req_hdr_size+key_len,len);
        if (server_cb->client_fd_list[client]->hash_table == NULL)
        {
          char* err_pack = make_packet(ERR_BAD_REQUEST, 0, NULL);
          send_msg(err_pack, res_hdr_size, client, server_cb);
          goto done;
        }
        key[key_len] = '\0';
        value[len] = '\0';
        put(server_cb->client_fd_list[client]->hash_table,
            key,
            value);
        goto REPLY_OK;
      }
      else
      {
        char* err_pack = make_packet(ERR_LENGTH_REQUIRED, 0, NULL);
        send_msg(err_pack, res_hdr_size, client, server_cb);
        free(err_pack);
        goto done;
      }
      break;
    case GET:
      if (server_cb->client_fd_list[client]->hash_table == NULL)
      {
        char* err_pack = make_packet(ERR_BAD_REQUEST, 0, NULL);
        send_msg(err_pack, res_hdr_size, client, server_cb);
        goto done;
      }
      char* result = NULL;
      char* key = malloc(key_len+1);
      memcpy(key, received_bytes+req_hdr_size, key_len);
      key[key_len] = '\0';
      int value_len =
        get(server_cb->client_fd_list[client]->hash_table,
          key,
          &result);
      char* pack = make_packet(OK, value_len, result);
      send_msg(pack, res_hdr_size+value_len, client, server_cb);
      goto done;
    default:;
      char* err_pack = make_packet(ERR_UNIMPLEMENTED, 0, NULL);
      send_msg(err_pack, res_hdr_size, client, server_cb);
      free(err_pack);
      goto done;
  }

REPLY_OK:;
  char* OK_pack = make_packet(OK, 0, NULL);
  send_msg(OK_pack, res_hdr_size, client, server_cb);
done:
  free(received_bytes);
  return;
}
