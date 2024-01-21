#include "Map.h"

#define HTTP_GET "GET"
#define HTTP_POST "POST"
#define HTTP_PUT "PUT"
#define HTTP_PATCH "PATCH"
#define HTTP_UPDATE "UPDATE"
#define HTTP_DELETE "DELETE"

typedef struct {
	int status_code;
	struct MapInfo* headerInfo;
	char status_message[50];
	char body[];
} Response;

struct sockaddr_in** dns_lookupIPv4(char addr[]);
Response* request(char addr[], char method[], struct MapInfo* mapinfo, char payload[]);
