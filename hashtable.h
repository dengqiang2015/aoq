#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define HTSIZE 1024
#define SUCCESS 1
#define FAILED 0
#define HASH_LEN 10

typedef struct _Bucket
{
	char *key;
	void **value;
	struct _Bucket *next;
} Bucket;

typedef struct _HashTable
{
	int size;
	int total;
	struct _Bucket *buckets;
} HashTable;

HashTable * ht;

int hash_init(HashTable **ht, size_t ht_size);
int hash_find(HashTable *ht, char *key, void **result);
int hash_insert(HashTable *ht, char *key, void *value);
int hash_remove(HashTable *ht, char *key);
int hash_loop(HashTable *ht, void **result);
int hash_index(HashTable *ht, char *key);
static unsigned int ELFHash(char *str, unsigned int length);
int hash_keys(HashTable *ht, char *keys, int *len);

