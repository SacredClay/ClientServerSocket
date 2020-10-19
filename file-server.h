/* This page contains a client program that can request a file from the server program
 * on the next page. The server responds by sending the whole file.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

#define SERVER_PORT 7243		/* arbitrary, but client & server must agree */
#define BUF_SIZE 4096		/* block transfer size */

void fatal(char *string)
{
  printf("%s\n", string);
  exit(1);
}
