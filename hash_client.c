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

/*****************************************************************************/
/******************************************************************* defines */
#define BUF 1024
#define _POSIX_C_SOURCE     200112L
#define DEFAULT_PORT_NBR    16000
/*****************************************************************************/
/****************************************************************** functions*/

/** @internal print usage of program
 *
 */
static void print_usage(void)
{
    printf("usage:\n\n");
    printf("    hash_client [-i IP] [-p port] [-h]\n");
}

/** @brief main function for client application
 *
 *  @param s The string to be printed.
 *  @param len The length of the string s.
 *  @return Void.
 */
int main (int argc, char **argv)
{
    int create_socket;
    char *buffer = malloc (BUF);
    struct sockaddr_in srv;
    int size;
    int option = 0;
    int iflag = 0, pflag = 0;

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

    /* create socket */
    if ((create_socket = socket(srv.sin_family, SOCK_STREAM, 0)) == -1) {
        perror("Error to create socket");
        exit(EXIT_FAILURE);
    }
    /* connect with server */
    if (connect(create_socket, (struct sockaddr *) &srv,
                sizeof(srv)) == -1) {
        close (create_socket);
        perror("Error to connect with server");
        exit(EXIT_FAILURE);
    }
    /* sucessfully connect with server */
    printf("*** Successfuly connect with server ***\n\n");

    /* enter string by string */
    do {
        size = recv(create_socket, buffer, BUF-1, 0);
        if( size > 0) {
            buffer[size] = '\0';
        }
        printf ("Hash code: %s\n", buffer);
        if (strcmp (buffer, "quit\n")) {
            printf ("Enter hash key: ");
            fgets (buffer, BUF, stdin);
            send(create_socket, buffer, strlen (buffer), 0);
        }
    } while (strcmp (buffer, "quit\n") != 0);
    /* close connection */
    close (create_socket);
    return EXIT_SUCCESS;
}

/*EOF*/
