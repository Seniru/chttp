typedef struct {
	char* key;
	char* value;
} Map;

struct MapInfo {
	Map* map;
	size_t size;
};

Map* map_create(size_t* size);
char* map_get(Map* src, char key[], size_t size);
void map_sort(Map* src, size_t size);
void map_insert(Map** dest, char key[], char value[], size_t* size);
