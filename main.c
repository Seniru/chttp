#include <stdio.h>
#include <arpa/inet.h>

#include "http.h"
#include "url.h"

int main() {
    size_t nheaders;
    Map* headers = map_create(&nheaders);
    map_insert(&headers, "Connection", "Keep-Alive", &nheaders);
	map_insert(&headers, "Content-Type", "text/json", &nheaders);

    struct MapInfo headerInfo;
    headerInfo.map = headers;
    headerInfo.size = nheaders;

    Response* resp = request("https://www.github.com/", HTTP_GET, &headerInfo, "{\"message\": \"test\"}");
    printf("Status code: %d\nStatus message: %s\n", resp->status_code, resp->status_message);
    struct MapInfo* respHeaderInfo = resp->headerInfo;
    printf("Header count: %lu\n", respHeaderInfo->size);
    printf("Headers: { ");
    for (int i = 0; i < respHeaderInfo->size; i++) {
        printf("%s: %s, ", respHeaderInfo->map[i].key, respHeaderInfo->map[i].value);
    }
    printf("}\nBody -----------------------------\n %s\n", resp->body);
    return 0;
}
