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


#include <errno.h>
#define DIE(assertion, call_description)				\
	do {								\
		if (assertion) {					\
			fprintf(stderr, "(%s, %d): ",			\
					__FILE__, __LINE__);		\
			perror(call_description);			\
			exit(errno);					\
		}							\
	} while (0)




#define IP_ADDR "34.241.4.235"
#define PORT 8080

#define PAYLOAD_LEN 200
#define COMMAND_LEN 50

int sockfd;


void register_account(int sockfd)
{
    sockfd = open_connection(IP_ADDR, PORT, AF_INET, SOCK_STREAM, 0);

    char *user = calloc(50, sizeof(char));
    char *pass = calloc(50, sizeof(char));

    printf("username: "); fflush(stdout); scanf("%s", user);
    printf("password: "); fflush(stdout); scanf("%s", pass);

    char **payload = calloc(1, sizeof(char*));
    payload[0] = calloc(PAYLOAD_LEN, sizeof(char));

    JSON_Value *value =  json_value_init_object();
    JSON_Object *object = json_value_get_object(value);

    json_object_set_string(object, "username", user);
    json_object_set_string(object, "password", pass);

    payload[0] = json_serialize_to_string_pretty(value);

   char *message = compute_post_request(IP_ADDR, "/api/v1/tema/auth/register", 
        "application/json", payload, 1, NULL, 0, NULL);

    send_to_server(sockfd, message);

    char *response = receive_from_server(sockfd);

    if (strstr(response, "error") != NULL) {
        printf("[ERROR] Username %s is taken!\n", user);
    } else printf("[SUCCES] Registration successful!\n");

    close_connection(sockfd);
}

char* access_library(int sockfd, char**cookies)
{
    
    if (cookies == NULL) { 
        printf("[ERROR] You have to login first!\n");
        return NULL;
    }

    sockfd = open_connection(IP_ADDR, PORT, AF_INET, SOCK_STREAM, 0);

    char *message = compute_get_request(IP_ADDR, "/api/v1/tema/library/access",
         NULL, cookies, 3, NULL);

    send_to_server(sockfd, message);
   
    char *response = receive_from_server(sockfd);

    response = strstr(response, ":\"");
    response = response + 2;
    response[strlen(response) - 2] = 0;

    printf("[SUCCES] Access approved!\n");

    close_connection(sockfd);

    return response;
}

void get_books(int sockfd, char *token)
{
    if (token == NULL) {
        printf("[ERROR] Oops, don't have access to library\n");
        return;
    }
    sockfd = open_connection(IP_ADDR, PORT, AF_INET, SOCK_STREAM, 0);

    char *message = compute_get_request(IP_ADDR, "/api/v1/tema/library/books",
         NULL, NULL, 0, token);
    

    send_to_server(sockfd, message);

    char *response = receive_from_server(sockfd);
    printf("%s\n", strstr(response, "["));

    close_connection(sockfd);


}

void add_book(int sockfd, char *token) {

    if (token == NULL) {
        printf("[ERROR] Oops, don't have access to library\n");
        return;
    }
    sockfd = open_connection(IP_ADDR, PORT, AF_INET, SOCK_STREAM, 0);

    char *title = calloc(50, sizeof(char));
    printf("title="); fflush(stdout); scanf("%s", title);

    char *author = calloc(50, sizeof(char));
    printf("author="); fflush(stdout); scanf("%s", author);

    char *genre = calloc(50, sizeof(char));
    printf("genre="); fflush(stdout); scanf("%s", genre);

    char *publisher = calloc(50, sizeof(char));
    printf("publisher="); fflush(stdout); scanf("%s", publisher);
    
    int page_count;
    printf("page_count="); fflush(stdout); scanf("%d", &page_count);

    JSON_Value *root_value = json_value_init_object();
    JSON_Object *root_object = json_value_get_object(root_value);
            
    char **payload = calloc(1, sizeof(char *));
    payload[0] = calloc(PAYLOAD_LEN, sizeof(char));
            
    json_object_set_string(root_object, "title", title);
    json_object_set_string(root_object, "author", author);
    json_object_set_string(root_object, "genre", genre);
    json_object_set_number(root_object, "page_count", page_count);
    json_object_set_string(root_object, "publisher", publisher);
    
    payload[0] = json_serialize_to_string_pretty(root_value);


    char *message = compute_post_request(IP_ADDR, "/api/v1/tema/library/books", 
        "application/json", payload, 1, NULL, 0, token);
    
    send_to_server(sockfd, message);

    char *response = receive_from_server(sockfd);

    if (strstr(response, "header is missing") != NULL) {
        printf("[ERROR] Authorization header is missing!\n");
    } else if (strstr(response, "error") != NULL) {
        printf("[ERROR] Bad Payload!\n");
    } else {
        printf("[SUCCESS] Book added!\n");
    }

    close_connection(sockfd);

}

void get_book(int sockfd, char *token)
{
    if (token == NULL) {
        printf("[ERROR] Oops, don't have access to library\n");
        return;
    }

    sockfd = open_connection(IP_ADDR, PORT, AF_INET, SOCK_STREAM, 0);

    int id;
    printf("id="); fflush(stdout); scanf("%d", &id);

    char *URL = calloc(PAYLOAD_LEN, sizeof(char));
    sprintf(URL, "/api/v1/tema/library/books/%d", id);

    char *message = compute_get_request(IP_ADDR, URL, NULL, NULL, 0, token);

    send_to_server(sockfd, message);

    char *response = receive_from_server(sockfd);

    if (strstr(response, "header is missing") != NULL) {
        printf("[ERROR] Authorization header is missing!\n");
    } else if (strstr(response, "No book was found") != NULL) {
        printf("[ERROR] The ID is not valid!\n");
    } else {
        response = basic_extract_json_response(response);
        response[strlen(response) - 1] = 0;
        printf("%s\n", response);
    }

    close_connection(sockfd);
}

char** login(int sockfd) 
{
    sockfd = open_connection(IP_ADDR, PORT, AF_INET, SOCK_STREAM, 0);

    char **cookies = calloc(3, sizeof(char*));
    for(int i = 0; i < 3; i++) {
        cookies[i] = calloc(100, sizeof(char));
    }

    char *aux;

    char *user = calloc(50, sizeof(char));
    char *pass = calloc(50, sizeof(char));

    printf("username: "); fflush(stdout); scanf("%s", user);
    printf("password: "); fflush(stdout); scanf("%s", pass);

    char **payload = calloc(1, sizeof(char*));
    payload[0] = calloc(PAYLOAD_LEN, sizeof(char));

    JSON_Value *value =  json_value_init_object();
    JSON_Object *object = json_value_get_object(value);

    json_object_set_string(object, "username", user);
    json_object_set_string(object, "password", pass);

    payload[0] = json_serialize_to_string_pretty(value);

    char *message = compute_post_request(IP_ADDR, "/api/v1/tema/auth/login", 
            "application/json", payload, 1, NULL, 0, NULL);

    send_to_server(sockfd, message);

    char *response = receive_from_server(sockfd);

    if (strstr(response, "error")  != NULL) {
        printf("[ERROR] Username or password is invalid!\n");
        return NULL;
    } else {
        aux = strstr(response, "connect");
        aux = strtok(aux, ";");
        cookies[0] = strdup(aux);
        cookies[1] = strdup(" Path=/");
        cookies[2] = strdup(" HttpOnly");
    }

    printf("[SUCCES] Login succes\n");

    close_connection(sockfd);

    return cookies;
}

void delete_book(int sockfd, char *token)
{
    if (token == NULL) {
        printf("[ERROR] Oops, don't have access to library!\n");
        return;
    }

    sockfd = open_connection(IP_ADDR, PORT, AF_INET, SOCK_STREAM, 0);

    int id;
    printf("id="); fflush(stdout); scanf("%d", &id);

    char *URL = calloc(PAYLOAD_LEN, sizeof(char));
    sprintf(URL, "/api/v1/tema/library/books/%d", id);

    char *message = compute_delete_request(IP_ADDR, URL, NULL, NULL, 0, token);

    send_to_server(sockfd, message);

    char *response = receive_from_server(sockfd);

    if (strstr(response, "header is missing") != NULL) {
        printf("[ERROR] Authorization header is missing!\n");
    } else if (strstr(response, "No book was found") != NULL) {
        printf("[ERROR] The ID is not valid!\n");
    } else {
        printf("[SUCCESS] Book deleted\n");
    }

    close_connection(sockfd);
}

void logout(int sockfd, char **cookies)
{
    if (cookies == NULL) {
        printf("[ERROR] You have to login first!\n");
        return;
    }

    sockfd = open_connection(IP_ADDR, PORT, AF_INET, SOCK_STREAM, 0);

    char *message = compute_get_request(IP_ADDR, "/api/v1/tema/auth/logout", NULL,
        cookies, 3, NULL);
    
    send_to_server(sockfd, message);

    char *response = receive_from_server(sockfd);
    if (strstr(response, "error") == NULL)
        printf("[SUCCES] Logged out\n");
    
}


int main(void) 
{
    char **cookies = NULL;
    char *token_JWT = NULL;


    char *command = calloc(COMMAND_LEN, sizeof(char));

    while (strcmp(command, "quit") != 0) {
        memset(command, 0, COMMAND_LEN);
        printf("command: "); fflush(stdout); scanf("%s", command);

        /* ================== REGISTER ================== */
        if (strcmp(command, "register")         == 0) {
            register_account(sockfd);
        }
        /* ================== LOGIN ================== */
        else if (strcmp(command, "login")       == 0) {
            cookies = login(sockfd);
        }
        /* ================== ACCESS ================== */
        else if (strcmp(command, "access")      == 0) {
            token_JWT = access_library(sockfd, cookies);
        }
        /* ================== GET BOOKS ================== */
        else if (strcmp(command, "books")       == 0) {
            get_books(sockfd, token_JWT);
        }
        /* ================== GET BOOK ================== */
        else if (strcmp(command, "book")        == 0) {
            get_book(sockfd, token_JWT);
        }
        /* ================== ADD BOOK ================== */
        else if (strcmp(command, "add_book")    == 0) {
            add_book(sockfd, token_JWT);
        }
        /* ================== DELETE BOOK ================== */
        else if (strcmp(command, "delete_book") == 0) {
            delete_book(sockfd, token_JWT);
        }
        /* ================== LOGOUT ================== */
        else if (strcmp(command, "logout")      == 0) {
            logout(sockfd, cookies);
            cookies = NULL;
        }
       
    }

    return 0;
}