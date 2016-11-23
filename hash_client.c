/**
 * @file hash_client.c
 * @author Fränz Ney (es16m013)
 * @date 1 Nov 2016
 * @brief File contains client functionallity for the hash cracker client
 *
 * @usage gcc -std=c99 -o hash_client hash_client.c -Wall -pedantic
 *        astyle -A3 --max-code-length=79 hash_client.c
 *
 */

/*****************************************************************************/
/****************************************************************** includes */
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <getopt.h>
#include <errno.h>
#include <signal.h>
#include <pthread.h>

/*****************************************************************************/
/******************************************************************* defines */
#define BUF 1024
#define _POSIX_C_SOURCE     200112L
#define DEFAULT_PORT_NBR    16000

/*****************************************************************************/
/******************************************************************* globals */
volatile sig_atomic_t run = 1;
volatile sig_atomic_t stop_wait = 1;

/*****************************************************************************/
/****************************************************************** functions*/

/** @brief thread to signal user that something is calculated!!
 *
 */
void *wait_signal(void *ptr){

  while(stop_wait == 1){
    system("tput civis");
    printf("\rwait .");
    fflush( stdout );
    sleep(1);
    printf("\rwait . .");
    fflush( stdout );
    sleep(1);
    printf("\rwait . . .");
    fflush( stdout );
    sleep(1);
    printf("\rwait      ");
    fflush( stdout );
    sleep(1);
  }
  printf("\r");
  fflush( stdout );
  system("tput cnorm");

  return NULL;
}

/** @brief ctrc handler
 *
 */
void cntrl_c_handler(int ignored) {

  run = 0;
}

/** @internal print usage of program
 *
 */
static void print_usage(void)
{

    printf("\nusage: hash_client [-i IP] [-p port] [-h]\n");
}

/** @brief main function for client application
 *
 */
int main (int argc, char **argv)
{
    int create_socket;
    char *buffer = malloc (BUF);
    struct sockaddr_in srv;
    int size;
    int option = 0;
    int iflag = 0, pflag = 0;
    pthread_t thread_wait;

    /* fill srv with null */
    memset(&srv, 0, sizeof(struct sockaddr_in));

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

    /* catch cntrl_c signal */
    signal(SIGINT, cntrl_c_handler);

    /* use default ipv4 address */
    if(iflag == 0) {
        srv.sin_family = AF_INET;
        inet_pton(AF_INET, "127.0.0.1", &srv.sin_addr);
    }
    /* use default port number */
    if(pflag == 0) {
        srv.sin_port = htons(DEFAULT_PORT_NBR);
    }

    /* create socket */
    if ((create_socket = socket(srv.sin_family, SOCK_STREAM, 0)) == -1) {
        perror("Error to create socket");
        exit(EXIT_FAILURE);
    }

    /* connect with server */
    if (connect(create_socket, (struct sockaddr *)&srv,
                sizeof(srv)) == -1) {
        close (create_socket);
        perror("Error to connect with server");
        exit(EXIT_FAILURE);
    }

    /* wait for ACK signal from server */
    while(1){
      size = recv(create_socket, buffer, BUF-1, 0);
      if( size > 0) {
        buffer[size] = '\0';
        if(strncmp(buffer, "ACK", 3) == 0){
          break;
        }
      }
    }

    /* Succesfully connect with server */
    printf("*** Successfuly connect with server ***\n\n");

    /* enter string by string */
    do {
      /* enter new data */
      printf (">> Enter hash key: ");
      fgets (buffer, BUF, stdin);
      send(create_socket, buffer, strlen (buffer), 0);

      /* start signal wait thread */
      stop_wait = 1;
      pthread_create(&thread_wait, NULL, wait_signal, (void *)NULL);

      /* wait for reply */
      size = recv(create_socket, buffer, BUF-1, 0);
      /* stop thread */
      stop_wait = 0;
      pthread_join(thread_wait, NULL);
      if( size > 0) {
        buffer[size] = '\0';
      }
      printf ("<< Hash code: %s\n", buffer);
    } while ((strcmp (buffer, "quit\n") != 0) && (run == 1));

    /* close connection */
    close (create_socket);
    return EXIT_SUCCESS;
}

/*EOF*/
