#include <stdlib.h>
#include <stdio.h>
#include <netdb.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "url.h"
#include "http.h"

#define MAX_IP_ADDRESS 10

struct sockaddr_in** dns_lookupIPv4(char addr[]) {

    struct sockaddr_in** pipv4 = malloc(sizeof(struct sockaddr_in) * MAX_IP_ADDRESS);

    struct addrinfo hints, *result, *rp;
    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_INET;
    hints.ai_socktype = 0;
    hints.ai_protocol = 0;
    
    int s = getaddrinfo(addr, NULL, &hints, &result);
    if (s != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(s));
        exit(EXIT_FAILURE);
    }
    
    int numAddr = 0;
    for (rp = result; rp != NULL; rp = rp->ai_next) {
        struct sockaddr_in *ipv4 = (struct sockaddr_in*) rp->ai_addr;
        pipv4[numAddr] = ipv4;
        numAddr++;
        if (numAddr > MAX_IP_ADDRESS) break;
    }
    freeaddrinfo(result);
    return pipv4;
}


struct sockaddr_in6** dns_lookupIPv6(char addr[]) {

    struct sockaddr_in6** pipv6 = malloc(sizeof(struct sockaddr_in6) * MAX_IP_ADDRESS);

    struct addrinfo hints, *result, *rp;
    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_INET6;
    hints.ai_socktype = 0;
    hints.ai_protocol = 0;
    
    int s = getaddrinfo(addr, NULL, &hints, &result);
    if (s != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(s));
        exit(EXIT_FAILURE);
    }
    
    int numAddr = 0;
    for (rp = result; rp != NULL; rp = rp->ai_next) {
        struct sockaddr_in6 *ipv6 = (struct sockaddr_in6*) rp->ai_addr;
        pipv6[numAddr] = ipv6;
        numAddr++;
        if (numAddr > MAX_IP_ADDRESS) break;
    }
    freeaddrinfo(result);
    return pipv6;
}

char* create_request_string(char host[], char method[], struct MapInfo* headerInfo, char path[], char payload[]) {
    size_t buffer_size = 1024;
    char* res = (char*) malloc(sizeof(char) * buffer_size);

    int written = sprintf(
        res,
        "%s /%s HTTP/1.1\r\nHost: %s\r\n",
        method,
        path,
        host        
    );
    // write the header information
    size_t nheaders = headerInfo->size;
    Map* headers = headerInfo->map;
    for (int i = 0; i < nheaders; i++) {
        char* h = (char*) malloc(sizeof(char) * 100);
        written = sprintf(h, "%s: %s\r\n", headers[i].key, headers[i].value);
        strcat(res, h);
        free(h);
    }
    strcat(res, "\r\n");
    strcat(res, payload);
    return res;
}

Response* parse_response_body(char response_buffer[]) {
    Response* resp = (Response*) malloc(sizeof(Response));
    size_t nheaders;
    Map* headers = map_create(&nheaders);

    int current_position = 0;
    char protocol[10];
    sscanf(response_buffer, "%s %d %[^\r\n]", protocol, &resp->status_code, resp->status_message);
    current_position += strlen(protocol) + strlen(resp->status_message) + 7;
    while (1) {
        char k[20];
        char v[1024];
        
        int read = sscanf(response_buffer + current_position, "%[^:]: %[^\r\n]\r\n", k, v);
        if (read != 2) break;
        map_insert(&headers, k, v, &nheaders);
        current_position += strlen(k) + strlen(v) + 4;
        if (strncmp("\r\n", response_buffer + current_position, 2) == 0) break;
    }
    struct MapInfo* headerInfo = (struct MapInfo*) malloc(sizeof(struct MapInfo*));
    headerInfo->size = nheaders;
    headerInfo->map = headers;
    resp->headerInfo = headerInfo;
    strcat(resp->body, response_buffer + current_position);
    return resp;
}

Response* request(char addr[], char method[], struct MapInfo* headerInfo, char payload[]) {
    struct URL* parsed_url = parse_url(addr);
    struct sockaddr_in** ipv4 = dns_lookupIPv4(parsed_url->host);

    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }
    struct sockaddr_in saddr = *ipv4[0];
    saddr.sin_port = htons(80);

    if (connect(sockfd, (struct sockaddr*) &saddr, sizeof(saddr)) == -1) {
        perror("Connection failed");
        exit(EXIT_FAILURE);
    }

    char* req = create_request_string(parsed_url->host, method, headerInfo, parsed_url->path, payload);
    if (send(sockfd, req, strlen(req) + 1, 0) == -1) {
        perror("Send failed");
        exit(EXIT_FAILURE);
    }
    free(req);
    
    int chunk_size = 1024;
    int total_received = 0;

    char* buffer = (char*) malloc(sizeof(char) * chunk_size);
    if (buffer == NULL) {
        perror("malloc failed");
        exit(EXIT_FAILURE);
    }

    while (1) {
        int received = recv(sockfd, buffer + total_received, chunk_size - total_received, 0);

        if (received == -1) {
            perror("Receive failed");
            free(buffer);
            return NULL;
        }

        if (received == 0) break;
        total_received += received;

        if (strstr(buffer, "\r\n\r\n") != NULL) break;

        // If the allocated buffer is too small, double its size
        if (total_received == chunk_size) {
            chunk_size *= 2;
            char* new_buffer = (char*)realloc(buffer, chunk_size * sizeof(char));
            if (new_buffer == NULL) {
                perror("realloc failed");
                free(buffer);
                exit(EXIT_FAILURE);
            }
            buffer = new_buffer;
        }
    }
    return parse_response_body(buffer);
}
