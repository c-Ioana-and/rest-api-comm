#include <stdlib.h>     /* exit, atoi, malloc, free */
#include <stdio.h>
#include <unistd.h>     /* read, write, close */
#include <string.h>     /* memcpy, memset */
#include <sys/socket.h> /* socket, connect */
#include <netinet/in.h> /* struct sockaddr_in, struct sockaddr */
#include <netdb.h>      /* struct hostent, gethostbyname */
#include <arpa/inet.h>
#include "helpers.h"
#include "requests.h"

char *compute_get_request(char *host, char *url, char *query_params,
                            char **cookies, int cookies_count, char *token)
{
    char *message = calloc(BUFLEN, sizeof(char));
    char *line = calloc(LINELEN, sizeof(char));

    // write the method name, URL, request params (if any) and protocol type
    if (query_params != NULL) {
        sprintf(line, "GET %s?%s HTTP/1.1", url, query_params);
    } else {
        sprintf(line, "GET %s HTTP/1.1", url);
    }

    compute_message(message, line);

    // add the host
    sprintf(line, "Host: %s", host);
    compute_message(message, line);

    // add headers and/or cookies, according to the protocol format
    if (cookies != NULL) {
        sprintf(line, "Cookie: ");
        for (int i = 0; i < cookies_count; i++) {
            strcat(line, cookies[i]);
            if (i != cookies_count - 1)
                strcat(line, "; ");
        }
        compute_message(message, line);
    }
    
    // check if token is not NULL
    if (token != NULL) {
        sprintf(line, "Authorization: Bearer %s", token);
        compute_message(message, line);
    }

    // add final new line
    compute_message(message, "");
    return message;
}

char *compute_post_request(char *host, char *url, char* content_type, char **body_data,
                            int body_data_fields_count, char **cookies, int cookies_count,
                            char *token)
{
    char *message = calloc(BUFLEN, sizeof(char));
    char *line = calloc(LINELEN, sizeof(char));
    char *body_data_buffer = calloc(LINELEN, sizeof(char));

    // write the method name, URL and protocol type
    sprintf(line, "POST %s HTTP/1.1", url);
    compute_message(message, line);
    
    // add the host
    sprintf(line, "Host: %s", host);
    compute_message(message, line);
    // add necessary headers (Content-Type and Content-Length are mandatory)
    //        in order to write Content-Length you must first compute the message size

    // calculte the lenght of the body_data and cookies
    int body_data_length = 0;
    for (int i = 0; i < body_data_fields_count; i++) {
        body_data_length += strlen(body_data[i]);
    }
    int cookies_length = 0;
    for (int i = 0; i < cookies_count; i++) {
        cookies_length += strlen(cookies[i]);
    }
    sprintf(line, "Content-Type: %s", content_type);
    compute_message(message, line);
    sprintf(line, "Content-Length: %d", body_data_length + cookies_length);
    compute_message(message, line);

    // Step 4 (optional): add cookies
    if (cookies != NULL) {
        sprintf(line, "Set-Cookie: ");
        for (int i = 0; i < cookies_count; i++) {
            strcat(line, cookies[i]);
            if (i != cookies_count - 1)
                strcat(line, "; ");
        }
        compute_message(message, line);
    }
    // check if token is not NULL
    if (token != NULL) {
        sprintf(line, "Authorization: Bearer %s", token);
        compute_message(message, line);
    }
    // add new line at end of header
    compute_message(message, "");

    // add data from body_data
    if (body_data != NULL) {
        char *temp = calloc(LINELEN, sizeof(char));
        memset(temp, 0, LINELEN);
        for (int i = 0; i < body_data_fields_count; i++) {
            strcat(temp, body_data[i]);
            if (i != body_data_fields_count - 1)
                strcat(temp, "&");
        }
        body_data_buffer[strlen(temp) - 1] = '\0';

        compute_message(message, temp);
    }
    compute_message(message, "");
    free(line);
    return message;
}

char *compute_delete_request(char *host, char *url, char *token)
{
    char *message = calloc(BUFLEN, sizeof(char));
    char *line = calloc(LINELEN, sizeof(char));

    // write the method name, URL, request params (if any) and protocol type
    sprintf(line, "DELETE %s HTTP/1.1", url);

    compute_message(message, line);

    // add the host
    sprintf(line, "Host: %s", host);
    compute_message(message, line);

    // check if token is not NULL
    if (token != NULL) {
        sprintf(line, "Authorization: Bearer %s", token);
        compute_message(message, line);
    }

    // add final new line
    compute_message(message, "");
    return message;
}

