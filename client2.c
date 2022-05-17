#include <stdio.h>      /* printf, sprintf */
#include <stdlib.h>     /* exit, atoi, malloc, free */
#include <unistd.h>     /* read, write, close */
#include <string.h>     /* memcpy, memset */
#include <sys/socket.h> /* socket, connect */
#include <netinet/in.h> /* struct sockaddr_in, struct sockaddr */
#include <netdb.h>      /* struct hostent, gethostbyname */
#include <arpa/inet.h>
#include "helpers.h"
#include "requests.h"
#include "parson.h"

#define IP_ADDR "34.241.4.235"
#define PORT 8080

int main(void) 
{
    char *response;
    int sockfd;

    sockfd = open_connection(IP_ADDR, PORT, AF_INET, SOCK_STREAM, 0);
    print("dasdassdasdsa\n");
    char *command = calloc(50, sizeof(char));
    
    while(strcmp(command, "exit") != 0) {
        if(strcmp(command, "register") == 0) {
            char *user = calloc(50, sizeof(char));
            char *pass = calloc(50, sizeof(char));

            printf("username:"); scanf("%s", user);
            printf("password:"); scanf("%s", pass);
        }
    }

    return 0;
}