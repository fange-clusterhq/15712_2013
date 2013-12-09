
#include <netinet/in.h>
#include <netinet/ip.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <sys/select.h>
#include <errno.h>

#include "common.h"
#include "handle_rpc.h"

#define BUF_SIZE 1024

int close_socket(int sock)
{
  if (close(sock))
  {
    fprintf(stderr, "Failed closing socket.\n");
    return 1;
  }
  return 0;
}


/* @brief check each fd in the readfds if it has data coming in
 * @param read_fds The fd set for reading.
 * @param max_fd The largest fd observed.
 * @param server_sock The fd server uses to listen
 * @return error codes in case any operation failed
 */

void check_readfds(fd_set* read_fds, struct server_cb* server_cb)
{
  /* uppon select() wake up, want to check each fd in the fd set */
  for(int i = 0; i < MAX_CLIENT; i++)
  {
    if (server_cb->client_fd_list[i] == NULL)
    {
      continue;
    }
    int current_client_fd = server_cb->client_fd_list[i]->client_fd;
    if (FD_ISSET(current_client_fd, read_fds))
    {
      /* make some buffer first */
      char* buf = malloc(BUF_SIZE);
      memset(buf, 0, BUF_SIZE);
      /* receive the byte */
      size_t byte_received =
        recv(current_client_fd, buf, BUF_SIZE, 0);
      /* error checking for byte receive */
      if (byte_received <= 0)
      {
        if (byte_received < 0)
          fprintf(stderr, "receiving error %d/n", (int)byte_received);
        server_cb->client_fd_list[i] = 0;
        close_socket(current_client_fd);
        free(buf);
        continue;
      }

      /* received some data, need to echo back,
       * so we need a new sent request
       */
      /* point of injection of part2 of p1 */
      /* 1. parse url
       * 2. check for error
       * 3. get the files
       * 4. get for more error
       * 5. prepare response
       * 6. add to queue
       * 7. done
       * */

      handle_receive(buf, byte_received, server_cb, i);

      /**********/
    }
  }
}


int main(int argc, char* argv[])
{
  int server_sock;
  struct sockaddr_in addr;
  /* read file descriptor list */
  fd_set read_fds;
  /* largest fd observed */
  int max_fd;

  /* parse commandline argument */
  int server_port = atoi(argv[1]);

  /* initialization for parameters used for select()*/
  max_fd = 0;
  FD_ZERO(&read_fds);
  /* end initialization */

  /* all networked programs must create a socket */
  if ((server_sock = socket(PF_INET, SOCK_STREAM, 0)) == -1)
  {
    fprintf(stderr, "Failed creating socket.\n");
    return EXIT_FAILURE;
  }

  addr.sin_family = AF_INET;
  addr.sin_port = htons(server_port);
  addr.sin_addr.s_addr = INADDR_ANY;

  /* servers bind sockets to ports---notify the OS they accept connections */
  if (bind(server_sock, (struct sockaddr *) &addr, sizeof(addr)) == -1)
  {
    close_socket(server_sock);
    fprintf(stderr, "Failed binding socket.\n");
    return EXIT_FAILURE;
  }


  if (listen(server_sock, 5) == -1)
  {
    close_socket(server_sock);
    fprintf(stderr, "Error listening on socket.\n");
    return EXIT_FAILURE;
  }

  /* set max_fd to be the listener, only socket known now */
  max_fd = server_sock;


  struct server_cb* server_cb = calloc_struct(struct server_cb);
  server_cb->server_sock = server_sock;
  server_cb->client_fd_list = calloc(sizeof(struct client_info*), max_num_client);

  while(1)
  {
    /* clear the read and write fd sets */
    FD_ZERO(&read_fds);
    /* add the listen socket to the read list */
    FD_SET(server_sock, &read_fds);
    /* add the client sockets to the read and write list */
    for(int i = 0; i < MAX_CLIENT; i++)
    {
      if (server_cb->client_fd_list[i] != NULL)
      {
        FD_SET(i, &read_fds);
        /* check what is the max fds */
        if (i > max_fd)
        {
          max_fd = i;
        }
      }
    }

    /* using select to multiplex concurrent connections */
    if((select(max_fd+1, &read_fds, NULL, NULL, NULL)) < 0)
    {
      fprintf(stderr, "Failed select() [%d].\n", errno);
      return EXIT_FAILURE;
    }


    /* select wake up
     * when select() is waked up, need to check each fd for data to be read
     * or received
     */

    /* check for new connections */
    if (FD_ISSET(server_sock,&read_fds))
    {
      /* socket used for client connection */
      int client_sock;
      if ((client_sock = accept(server_sock, NULL, NULL)) == -1)
      {
        fprintf(stderr, "Error accepting connection.\n");
        continue;
      }
      server_cb->client_fd_list[client_sock] = calloc(sizeof(struct client_info), 1);
      server_cb->client_fd_list[client_sock]->client_fd = client_sock;
    }

    /* check readfds */
    check_readfds(&read_fds, server_cb);
  }
  return EXIT_SUCCESS;
}

