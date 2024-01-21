#include <stdlib.h>
#include <stdio.h>
#include "url.h"

struct URL* parse_url(char* url) {
    struct URL* result = (struct URL*) malloc(sizeof(struct URL));
    sscanf(url, "%[^:]://%[^/]/%[^\n]", result->protocol, result->host, result->path);
    return result;
}
