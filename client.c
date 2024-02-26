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
#include "poll.h"
#include "ctype.h"

char ip[13] = "34.254.242.81";

// functie care verifica daca un string contine caractere care nu sunt alfanumerice
// sau semne de punctuatie
int is_valid(char *str) {
    for (int i = 0; i < strlen(str) - 1; i++) {
        if (!isalnum(str[i]) && !ispunct(str[i]))
            return 0;
    }
    return 1;
}

int is_valid_or_space(char *str) {
    if (str == NULL || strcmp(str, "\n") == 0)
        return 0;
    for (int i = 0; i < strlen(str) - 1; i++) {
        if (!isalnum(str[i]) && !ispunct(str[i]) && !isspace(str[i]))
            return 0;
    }
    return 1;
}

// functie care inregistreaza un cont nou pe server
void register_account (char *user, char *pass, int svsock) {
    // verific daca username-ul sau parola contin alte caractere in afara de cele
    // alfanumerice sau semne de punctuatie
    if (is_valid(user) == 0 || is_valid(pass) == 0) {
        printf("415 - Invalid username or password!\n");
        return;
    }

    JSON_Value *root_value = json_value_init_object();
    JSON_Object *json_object = json_value_get_object(root_value);
    json_object_set_string(json_object, "username", user);
    json_object_set_string(json_object, "password", pass);

    char *serialized_string = json_serialize_to_string_pretty(root_value);

    char *message = compute_post_request(ip, "/api/v1/tema/auth/register",
                "application/json", &serialized_string, 1, NULL, 0, NULL);
    send_to_server(svsock, message);
    char *response = receive_from_server(svsock);

    if (strstr(response, "is taken") != NULL)
        printf("Username is taken!\n");
    else printf("201 - OK - Utilizator înregistrat cu succes!\n");
}

// functie care autentifica un cont pe server, intorcand cookie-ul de sesiune
char *login_account (char *user, char *pass, int svsock) {
    // verific daca username-ul sau parola contin alte caractere in afara de cele
    // alfanumerice sau semne de punctuatie
    if (is_valid(user) == 0 || is_valid(pass) == 0) {
        printf("415 - Invalid username or password!\n");
        return "WRONG";
    }

    JSON_Value *root_value = json_value_init_object();
    JSON_Object *json_object = json_value_get_object(root_value);
    json_object_set_string(json_object, "username", user);
    json_object_set_string(json_object, "password", pass);

    char *serialized_string = json_serialize_to_string_pretty(root_value);
    char *message = compute_post_request(ip, "/api/v1/tema/auth/login",
            "application/json", &serialized_string, 1, NULL, 0, NULL);
    send_to_server(svsock, message);
    char *response = receive_from_server(svsock);

    if (strstr(response, "400 Bad Request") != NULL) {
        printf("400 - Wrong login credentials!\n");
        return "WRONG";
    }

    char *cookie = strstr(response, "connect.sid=");
    cookie = strtok(cookie, ";");
    printf("200 - OK - Successfully logged in!\n");
    return cookie;
}

char *req_access (char *cookie, int svsock) {
    char *url = "/api/v1/tema/library/access";

    char *message = compute_get_request(ip, url, NULL, &cookie, 1, NULL);
    send_to_server(svsock, message);
    char *response = receive_from_server(svsock);

    if (strstr(response, "200 OK") != NULL) {
        // get the token
        char *token = strstr(response, "token");
        token = strtok(token, ":");
        token = strtok(NULL, ":");

        token = strtok(token, "\"");
        printf("200 - OK - Token : %s\n", token);
        return token;
    } else {
        printf("403 - Forbidden - Unauthorized access!\n");
        return "WRONG";
    }
}

void show_books(char *token, int svsock) {
    char *url = "/api/v1/tema/library/books";

    char *message = compute_get_request(ip, url, NULL, NULL, 0, token);
    send_to_server(svsock, message);
    char *response = receive_from_server(svsock);

    // parse the list of books
    char *book_list = strstr(response, "[");
    JSON_Value *root_value = json_parse_string(book_list);
    JSON_Array *books = json_value_get_array(root_value);

    // show all books in the list
    if (json_array_get_count(books) == 0) {
        printf("No books in the library!\n");
        return;
    }
    else {
        printf("[\n");
        for (int i = 0; i < json_array_get_count(books); i++) {
            JSON_Object *book = json_array_get_object(books, i);
            printf("\t{\n");
            printf("\t\tid: %d,\n", (int)json_object_get_number(book, "id"));
            printf("\t\ttitle: %s\n", json_object_get_string(book, "title"));
            printf("\t}\n");
        }
        printf("]\n");
    }
}

// functie care afiseaza date despre o carte
void get_book(int id, char *token, int svsock) {
    char *url = calloc(LINELEN, sizeof(char));
    sprintf(url, "/api/v1/tema/library/books/%d", id);

    char *message = compute_get_request(ip, url, NULL, NULL, 0, token);
    send_to_server(svsock, message);
    char *response = receive_from_server(svsock);

    if (strstr(response, "404 Not Found") != NULL) {
        printf("404 - Not Found - Book not found!\n");
        return;
    }

    // parse the book
    char *book = strstr(response, "{");
    JSON_Value *root_value = json_parse_string(book);
    JSON_Object *book_obj = json_value_get_object(root_value);

    if (book_obj == NULL) {
        printf("Ups? Try again...\n");
        return;
    }
    // show the book
    printf("id = %d\n", (int)json_object_get_number(book_obj, "id"));
    printf("title = %s\n", json_object_get_string(book_obj, "title"));
    printf("author = %s\n", json_object_get_string(book_obj, "author"));
    printf("publisher = %s\n", json_object_get_string(book_obj, "publisher"));
    printf("genre = %s\n", json_object_get_string(book_obj, "genre"));
    printf("page_count = %d\n", (int)json_object_get_number(book_obj, "page_count"));
}

// functie care adauga o carte noua, trimitand cerere POST
void add_book(char *title, char *author, char *genre, char *publisher,
                        int page_count, char *token, int svsock) {
    if (is_valid_or_space(title) == 0 || is_valid_or_space(author) == 0 ||
        is_valid_or_space(genre) == 0 || is_valid_or_space(publisher) == 0) {
        printf("415 - Invalid book format!\n");
        return;
    }

    if (page_count <= 0) {
        printf("403 - Invalid page count!\n");
        return;
    }
    char *url = "/api/v1/tema/library/books";

    JSON_Value *root_value = json_value_init_object();
    JSON_Object *json_object = json_value_get_object(root_value);

    // evit \n de la sfarsitul fiecarui string citit
    title[strlen(title) - 1] = '\0';
    author[strlen(author) - 1] = '\0';
    genre[strlen(genre) - 1] = '\0';
    publisher[strlen(publisher) - 1] = '\0';

    json_object_set_string(json_object, "title", title);
    json_object_set_string(json_object, "author", author);
    json_object_set_string(json_object, "genre", genre);
    json_object_set_string(json_object, "publisher", publisher);
    json_object_set_number(json_object, "page_count", page_count);

    char *serialized_string = json_serialize_to_string_pretty(root_value);

    char *message = compute_post_request(ip, url,
                "application/json", &serialized_string, 1, NULL, 0, token);
    send_to_server(svsock, message);
    char *response = receive_from_server(svsock);

    if (strstr(response, "200 OK") != NULL)
        printf("200 - OK - Book added successfully!\n");
    else printf("400 - Bad Request - Invalid book format!\n");
}

// functie care sterge o carte, trimitand cerere DELETE
void delete_book(int id, char *token, int svsock) {
    char *url = calloc(LINELEN, sizeof(char));
    sprintf(url, "/api/v1/tema/library/books/%d", id);

    char *message = compute_get_request(ip, url, NULL, NULL, 0, token);
    send_to_server(svsock, message);
    char *response = receive_from_server(svsock);

    // check if book exists
    if (strstr(response, "404 Not Found") != NULL) {
        printf("404 - Not Found - Book not found!\n");
        return;
    }
    // delete the book
    message = compute_delete_request(ip, url, token);
    send_to_server(svsock, message);
    response = receive_from_server(svsock);

    if (strstr(response, "200 OK") != NULL)
        printf("200 - OK - Book deleted successfully!\n");
    else if (strstr(response, "204 No Content") != NULL)
        printf("204 - No content.\n");
    else if (strstr(response, "202 Accepted") != NULL)
        printf("202 - Accepted, but non-comittal.\n");
}

// functie care trimite cerere GET pentru logout
void get_logout(char *cookie, int svsock) {
    char *url = "/api/v1/tema/auth/logout";

    char *message = compute_get_request(ip, url, NULL, &cookie, 1, NULL);
    send_to_server(svsock, message);
    char *response = receive_from_server(svsock);

    if (strstr(response, "200 OK") != NULL)
        printf("200 - OK - Successfully logged out!\n");
    else printf("403 - Forbidden - Unauthorized access!\n");
}

int main(int argc, char *argv[]) {
    int sockfd;

    char *current_cookie = NULL;
    char *current_token = NULL;

    if (argc != 1) {
        printf("Usage: ./client\n");
        exit(0);
    }

    // deschid socket-ul
    struct sockaddr_in my_addr;
    memset(&my_addr, 0, sizeof(struct sockaddr_in));
    inet_aton(ip, &(my_addr.sin_addr));
    my_addr.sin_family = AF_INET;
    my_addr.sin_port = htons(8080);

    // citeste comenzi de la tastatura
    while (1) {
        sockfd = open_connection(ip, 8080, AF_INET, SOCK_STREAM, 0);

        char *command = calloc(LINELEN, sizeof(char));
        fgets(command, LINELEN, stdin);
        if (strncmp(command, "register", 8) == 0) {
            char *user, *pass;
            user = calloc(LINELEN, sizeof(char));
            pass = calloc(LINELEN, sizeof(char));

            // citeste user si pass de la tastatura
            printf("username=");
            fgets(user, LINELEN, stdin);
            printf("password=");
            fgets(pass, LINELEN, stdin);

            // trimit cerere de inregistrare
            register_account(user, pass, sockfd);
            free(user);
            free(pass);
        } else if (strncmp(command, "login", 5) == 0) {
            if (current_cookie != NULL) {
                printf("403 - Already logged in!\n");
                continue;
            }
            char *user, *pass;
            user = calloc(LINELEN, sizeof(char));
            pass = calloc(LINELEN, sizeof(char));

            printf("username=");
            fgets(user, LINELEN, stdin);
            printf("password=");
            fgets(pass, LINELEN, stdin);

            // trimit cerere de autentificare, obtin un cookie
            current_cookie = login_account(user, pass, sockfd);

            if (strcmp(current_cookie, "WRONG") == 0)
                current_cookie = NULL;
            free(user);
            free(pass);
        } else if (strncmp(command, "enter_library", 13) == 0) {
            if (current_cookie == NULL) {
                printf("401 - Not logged in!\n");
                continue;
            }
            // trimit cerere de acces la biblioteca, obtin token
            current_token = req_access(current_cookie, sockfd);
        } else if (strncmp(command, "get_books", 9) == 0) {
            if (current_token == NULL) {
                printf("403 - Unauthorized access! Try asking for it first...\n");
                continue;
            }
            show_books(current_token, sockfd);
        } else if (strncmp(command, "get_book", 8) == 0) {
            if (current_cookie == NULL) {
                printf("401 - Not logged in!\n");
                continue;
            }
            if (current_token == NULL) {
                printf("403 - Unauthorized access! Try asking for it first...\n");
                continue;
            }
            printf("id=");
            char id[10];
            fgets(id, LINELEN, stdin);
            int id_int = strtol(id, NULL, 10);

            get_book(id_int, current_token, sockfd);
        } else if (strncmp(command, "add_book", 8) == 0) {
            if (current_cookie == NULL) {
                printf("401 - Not logged in!\n");
                continue;
            }
            if (current_token == NULL) {
                printf("403 - Unauthorized access! Try asking for it first...\n");
                continue;
            }
            printf("title = ");
            char *title = calloc(LINELEN, sizeof(char));
            fgets(title, LINELEN, stdin);

            printf("author = ");
            char *author = calloc(LINELEN, sizeof(char));
            fgets(author, LINELEN, stdin);

            printf("genre = ");
            char *genre = calloc(LINELEN, sizeof(char));
            fgets(genre, LINELEN, stdin);

            printf("publisher = ");
            char *publisher = calloc(LINELEN, sizeof(char));
            fgets(publisher, LINELEN, stdin);

            printf("page_count = ");
            char page_count[10];
            fgets(page_count, LINELEN, stdin);
            int page_count_int = strtol(page_count, NULL, 10);

            add_book(title, author, genre, publisher, page_count_int, current_token, sockfd);
            free(title);
            free(author);
            free(genre);
            free(publisher);
        }  else if (strncmp(command, "delete_book", 11) == 0) {
            if (current_cookie == NULL) {
                printf("401 - Not logged in!\n");
                continue;
            }
            if (current_token == NULL) {
                printf("403 - Unauthorized access! Try asking for it first...\n");
                continue;
            }
            printf("id=");
            char id[10];
            fgets(id, LINELEN, stdin);

            int id_int = strtol(id, NULL, 10);

            delete_book(id_int, current_token, sockfd);
        } else if (strncmp(command, "logout", 6) == 0) {
            if (current_cookie != NULL)
                get_logout(current_cookie, sockfd);
            else printf("401 - Not logged in!\n");
            current_cookie = NULL;
            current_token = NULL;
        }  else if (strncmp(command, "exit", 4) == 0) {
            printf("Exiting...\n");
            close_connection(sockfd);
            return 0;
        } else printf("Invalid command!\n");
    }

    close_connection(sockfd);
    return 0;
}
