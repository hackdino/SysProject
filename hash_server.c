/**
 * @file hash_server.c
 * @author Fränz Ney (es16m013)
 * @date 19 Nov 2016
 * @brief File contains server functionallity for the hash cracker client
 *
 * @usage gcc hash_server.c crc32.c -o hash_server -Wall
 *                          -pedantic-errors -lpthread
 *
 *        astyle -A3 --max-code-length=70 --indent=spaces=2 hash_server.c
 *
 */

/*****************************************************************************/
/****************************************************************** includes */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/time.h>
#include <getopt.h>
#include <pthread.h>
#include <inttypes.h>
#include <time.h>
#include "crc32.h"

/*****************************************************************************/
/******************************************************************* defines */
#define TRUE   1
#define FALSE  0
#define DEFAULT_PORT_NBR    16000

/*****************************************************************************/
/******************************************************************** typedef*/
typedef struct thread_job_s {
  char data[1024];
  int len;
  int socket_fd;
} thread_job_t;

/*****************************************************************************/
/******************************************************************* globals */

/** @brief crc thread
 *
 */
void *hash_cracker(void*ptr)
{

  thread_job_t *job = (thread_job_t *)ptr;
  uint32_t orig_crc = crc32(job->data, job->len);
  char result[1024];
  uint32_t cur_crc;
  uint32_t i = 0;

  /* search for equal hash code */
  while (1) {
    uint8_t in[] = {(i>>24), (i>>16), (i>>8), i};
    cur_crc = crc32(in, sizeof(in));

    if (cur_crc == orig_crc) {
      break;
    }
    i++;
  }

  /* format result in string */
  sprintf(result, "0x%08"PRIx32"\r\n", i);

  /* send result to client */
  if(send(job->socket_fd, result, strlen(result),
          0) != strlen(result) ) {
    perror("send");
  }

  return NULL;
}

/** @internal print usage of program
 *
 */
static void print_usage(void)
{

  printf("\nusage: hash_server [-i IP] [-p port] [-h]\n");
}

/** @brief main function for server application
 *
 */
int main(int argc , char *argv[])
{
  int master_socket , addrlen , new_socket , client_socket[30] ,
      max_clients = 30 , activity, i , valread , sd;
  int max_sd;
  int option = 0;
  int iflag = 0, pflag = 0;
  pthread_t thread[30];
  thread_job_t job[30];
  struct sockaddr_in srv;
  char buffer[1025];  //data buffer of 1K
  //set of socket descriptors
  fd_set readfds;
  //a message
  char *message = "ACK\r\n";

  /* decode arguments */
  while ((option = getopt(argc, argv,"i:p:h")) != -1) {
    switch (option) {
    case 'i' :
      if(inet_pton(AF_INET, optarg, &srv.sin_addr)) {
        /* valid ipv4 address */
        srv.sin_family = AF_INET;
        iflag = 1;
      } else if(inet_pton(AF_INET6 , optarg, &srv.sin_addr)) {
        /* valid ipv6 address */
        srv.sin_family = AF_INET6;
        printf("ipv6\n");
        iflag = 1;
      } else {
        /* No valid ipv6 or ipv4 address */
        errno = EINVAL;
        perror("No valid IPv4 or Ipv6 address");
        exit(EXIT_FAILURE);
      }
      break;
    case 'p' :
      srv.sin_port = htons(atoi(optarg));
      pflag = 1;
      break;
    case 'h' :
      print_usage();
      exit(EXIT_FAILURE);
    default:
      print_usage();
      exit(EXIT_FAILURE);
      break;
    }
  }

  /* use default ipv4 address */
  if(iflag == 0) {
    srv.sin_family = AF_INET;
    inet_pton(AF_INET, "127.0.0.1", &srv.sin_addr);
  }
  /* use default port number */
  if(pflag == 0) {
    srv.sin_port = htons(DEFAULT_PORT_NBR);
  }

  /* initialise all client_socket[] to 0 so not checked */
  for (i = 0; i < max_clients; i++) {
    client_socket[i] = 0;
  }

  /* create a master socket */
  if((master_socket = socket(srv.sin_family, SOCK_STREAM , 0)) == -1) {
    perror("Error to create socket");
    exit(EXIT_FAILURE);
  }

  /* bind socket */
  if (bind(master_socket, (struct sockaddr *)&srv,
           sizeof(srv))<0) {
    perror("bind failed");
    exit(EXIT_FAILURE);
  }
  printf("*** Hash cracker server is ready ***\n\n");

  /* max 3 pending connections for master_socket */
  if (listen(master_socket, 3) < 0) {
    perror("listen");
    exit(EXIT_FAILURE);
  }

  /* accept the incoming connection */
  addrlen = sizeof(srv);
  puts(">> Waiting for connections ...");

  while(TRUE) {
    /* clear the socket set */
    FD_ZERO(&readfds);

    /* add master socket to set */
    FD_SET(master_socket, &readfds);
    max_sd = master_socket;

    /* add child sockets to set */
    for ( i = 0 ; i < max_clients ; i++) {
      /* socket descriptor */
      sd = client_socket[i];

      /* if valid socket descriptor then add to read list */
      if(sd > 0) {
        FD_SET( sd , &readfds);
      }

      /* get highest file desciptor => need for select() */
      if(sd > max_sd) {
        max_sd = sd;
      }
    }

    /* wait for an activity on one of the sockets */
    activity = select(max_sd + 1, &readfds, NULL, NULL, NULL);

    if((activity < 0) && (errno != EINTR)) {
      printf("select error");
    }

    /* incomming connection on master_sockets */
    if(FD_ISSET(master_socket, &readfds)) {
      if((new_socket = accept(master_socket, (struct sockaddr *)&srv,
                              (socklen_t*)&addrlen)) < 0) {
        perror("accept");
        exit(EXIT_FAILURE);
      }

      /* inform user about new connection */
      printf(">> New connection , socket fd is %d , ip is : %s , port : %d \n",
             new_socket , inet_ntoa(srv.sin_addr) , ntohs(srv.sin_port));

      /* send ACK to new connection */
      if( send(new_socket, message, strlen(message),
               0) != strlen(message) ) {
        perror("send");
      }

      /* add new socket to array of sockets */
      for (i = 0; i < max_clients; i++) {
        /* if position is empty */
        if( client_socket[i] == 0 ) {
          client_socket[i] = new_socket;
          printf("Adding to list of sockets as %d\n" , i);
          break;
        }
      }
    }
    /* incomming connection on other socket */
    for(i = 0; i < max_clients; i++) {
      sd = client_socket[i];
      if(FD_ISSET(sd, &readfds)) {
        //Check if it was for closing , and also read the incoming message
        if((valread = read(sd, buffer, 1024)) == 0) {
          /* Somebody disconnected , get his details and print */
          getpeername(sd , (struct sockaddr*)&srv , (socklen_t*)&addrlen);
          printf("Host disconnected , ip %s , port %d \n" ,
                 inet_ntoa(srv.sin_addr) ,
                 ntohs(srv.sin_port));
          /* Close the socket and mark as 0 in list for reuse */
          close(sd);
          client_socket[i] = 0;
        } else {
          /* copy data to job */
          memcpy(job[i].data, buffer, valread);
          job[i].socket_fd = sd;
          job[i].len = valread;
          pthread_create(&thread[i], NULL, hash_cracker, (void *)&job[i]);
        }
      }
    }
  }
  return 0;
}
