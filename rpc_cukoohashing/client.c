/******************************************************************************
 * echo_client.c                                                               *
 *                                                                             *
 * Description: This file contains the C source code for an echo client.  The  *
 *              client connects to an arbitrary <host,port> and sends input    *
 *              from stdin.                                                    *
 *                                                                             *
 * Authors: Athula Balachandran <abalacha@cs.cmu.edu>,                         *
 *          Wolf Richter <wolf@cs.cmu.edu>                                     *
 *                                                                             *
 *******************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <netinet/in.h>
#include <netinet/ip.h>

#include <assert.h>

#include "common.h"

#define ECHO_PORT 7080
#define BUF_SIZE 4096

int sock;

struct result result;

char* make_request(int function, char* key, int len, char* payload)
{
  struct request_hdr header;
  header.function = function;
  header.key_length = strlen(key);
  header.value_length = len;
  char* pack = malloc(sizeof(header)+len+strlen(key));
  memcpy(pack, &header, sizeof(header));
  memcpy(pack+sizeof(header), key, strlen(key));
  memcpy(pack+sizeof(header)+strlen(key), payload, len);
  return pack;
}

void send_msg(char* msg, int size, int client_fd)
{
  size_t byte_sent =
    send(client_fd, msg, size, 0);
  if (byte_sent <= 0)
  {
    fprintf(stderr, "send failed");
    close(client_fd);
  }
}



void handle_response(int function)
{
  char buf[BUF_SIZE];
  memset(buf, 0, BUF_SIZE);
  int ret = recv(sock, buf, BUF_SIZE, 0);
  if (ret < 0)
  {
    result.err = ERR_INTERNAL;
    return;
  }
  struct response_hdr header;
  memcpy(&header, buf, res_hdr_size);
  switch(function)
  {
    case INIT_HASH_TABLE:
    case PUT:
      result.err = header.error_code;
      return;
    case GET:
      result.err = header.error_code;
      if (result.err == OK)
      {
        char* tmp = malloc(header.content_length);
        memcpy(tmp, buf+res_hdr_size, header.content_length);
        result.payload = tmp;
      }
      return;
    default:
      assert(0);
  }
  return;
}

void init_hashtable()
{
  char* request;
  request = make_request(INIT_HASH_TABLE, "", 0, "");
  send_msg(request, req_hdr_size, sock);
  handle_response(INIT_HASH_TABLE);
  if (result.err != OK)
  {
    perror("failed init_hash table init\n");
  }
  return;
}

void put(char* key, char* value)
{
  char* request;
  request = make_request(PUT, key, strlen(value), value);
  send_msg(request, req_hdr_size + strlen(value)+strlen(key), sock);
  handle_response(PUT);
  if (result.err != OK)
  {
    perror("failed put\n");
  }
  return;
}

void get(char* key)
{
  char* request;
  request = make_request(GET, key, 0, "");
  send_msg(request, req_hdr_size+strlen(key), sock);
  handle_response(GET);
  if (result.err != OK || result.payload == NULL)
  {
    perror("failed get\n");
  }
  else
  {
    printf("response:\n%s\n", result.payload);
  }
  return;
}



void run_client()
{
  init_hashtable();
  put("Yihua Fang", "yihuaf");
  put("Xiaobo Zhao", "xiaoboz");
  put("Anuj", "Anuj_123");
  get("Yihua Fang");
  get("Xiaobo Zhao");
  get("Anuj");
  return;
}

int main(int argc, char* argv[])
{
  if (argc != 3)
  {
    fprintf(stderr, "usage: %s <server-ip> <port>",argv[0]);
    return EXIT_FAILURE;
  }


  int status;
  struct addrinfo hints;
  memset(&hints, 0, sizeof(struct addrinfo));
  struct addrinfo *servinfo; //will point to the results
  hints.ai_family = AF_UNSPEC;  //don't care IPv4 or IPv6
  hints.ai_socktype = SOCK_STREAM; //TCP stream sockets
  hints.ai_flags = AI_PASSIVE; //fill in my IP for me

  if ((status = getaddrinfo(argv[1], argv[2], &hints, &servinfo)) != 0)
  {
    fprintf(stderr, "getaddrinfo error: %s \n", gai_strerror(status));
    return EXIT_FAILURE;
  }

  if((sock = socket(servinfo->ai_family, servinfo->ai_socktype, servinfo->ai_protocol)) == -1)
  {
    fprintf(stderr, "Socket failed");
    return EXIT_FAILURE;
  }

  if (connect (sock, servinfo->ai_addr, servinfo->ai_addrlen) == -1)
  {
    fprintf(stderr, "Connect");
    return EXIT_FAILURE;
  }

  run_client();

  freeaddrinfo(servinfo);
  close(sock);
  return EXIT_SUCCESS;
}
