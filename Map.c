#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "Map.h"

int cmp_fn(const void* lhs, const void* rhs) {
	return strcmp( ((Map*)lhs)->key, ((Map*)rhs)->key );
}

Map* map_create(size_t* size) {
	Map* map = (Map*) malloc(sizeof(Map));
	if (map == NULL) {
		fprintf(stderr, "malloc failed\n");
		exit(EXIT_FAILURE);
	}
	*size = 0;
	return map;
}

void map_sort(Map* src, size_t size) {
	qsort(src, size, sizeof(src[0]), cmp_fn);
}

void map_insert(Map** dest, char key[], char value[], size_t* size) {
	if ((*size) > 0) {
		// check if the key alreay exists
		for (int i = 0; i < *size; i++) {
			if (strcmp((*dest)[i].key, key) == 0) {
				(*dest)[i].value = value;
				return;
			}
		}
		// key doesn't exist, allocate memory for a new key value pair
		Map* new_map = (Map*) realloc(*dest, sizeof(Map) * (*size + 1));
		if (new_map == NULL) {
			fprintf(stderr, "realloc failed\n");
			exit(EXIT_FAILURE);
		}
		*dest = new_map;	
	}
	(*dest)[*size].key = strdup(key);
	(*dest)[*size].value = strdup(value);
	(*size)++;
	map_sort(*dest, *size);
}

char* map_get(Map* src, char key[], size_t size) {

	if (size == 0) {
		return NULL;
	}
	Map searchfor = { .key = key };
	Map* matched = bsearch(&searchfor, src, size, sizeof(src[0]), cmp_fn);
	if (matched == NULL) {
		return NULL;
	}
	return matched->value;
}
