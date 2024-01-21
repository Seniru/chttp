struct URL {
    char protocol[10];
    char host[50];
    char path[100];
};

struct URL* parse_url(char* url);
