#ifndef _INCLUDE_HASHTABLE_H 
#define _INCLUDE_HASHTABLE_H 1  
#include "hashtable.h"
#endif
int hash_init(HashTable **ht, size_t ht_size) {

    if (NULL == ht) {
        exit(1);
    }
    (*ht)->size = ht_size;
    (*ht)->total = 0;
    Bucket *bucket = (Bucket *)malloc(sizeof(Bucket) * ht_size);
    (*ht)->buckets = bucket;

    for(ht_size; ht_size>0; ht_size--)
    {   
        bucket->key = NULL;
        bucket->value = NULL;
        bucket->next = NULL;
        bucket++;
    }

    return SUCCESS;
}

int hash_insert(HashTable *ht, char *key, void *value) {

    if (ht->total >= ht->size) {
        //ht->size = ht->size*2;
        //ht->buckets = (Bucket *)realloc(ht->buckets, sizeof(Bucket) * (ht->size));
		return FAILED;
    }
    int index = hash_index(ht, key);
    Bucket *bucket = &ht->buckets[index];
    int _tmpindex;
    char _tmpindexstr[20];

   while (0 != bucket->value) {
    
        while (NULL != bucket->next) {
            if (strcmp(key, bucket->key) == 0) {
               *(bucket->value) = value;
                return SUCCESS;
            }
            bucket = bucket->next;
        }

        do {
            _tmpindex = abs(rand() - index);
            sprintf(_tmpindexstr, "%d", _tmpindex);
            _tmpindex = hash_index(ht, _tmpindexstr);
        } while (_tmpindex == index || ht->buckets[_tmpindex].value != NULL);

        index = _tmpindex;
        bucket->next = &ht->buckets[index];
     
       bucket = bucket->next;
    }
 

    bucket->key = (char *)malloc(sizeof(key));
    bucket->value = (void **)calloc(1, sizeof(void *));
    memcpy(bucket->key, key, sizeof(key));
    *(bucket->value) = value;
    bucket->next = NULL;
    ht->total++;
    return SUCCESS;
}

int hash_find(HashTable *ht, char *key, void **result) {
    int index = hash_index(ht, key);

    Bucket *bucket = &ht->buckets[index];
    if (NULL == bucket->value) {
 
        return FAILED;
    }

    while (strcmp(key, bucket->key)) {
        if (NULL != bucket->next) {
            bucket = bucket->next;
        } else {
            break;
        }
    }
    
    if (NULL == *(bucket->value) || strcmp(key, bucket->key)) {
    
        return FAILED;
    }

    *result = *(bucket->value);
    return SUCCESS;

}

int hash_delete(HashTable *ht, char *key) {
    int index = hash_index(ht, key);
    Bucket *bucket = &ht->buckets[index];
    if (NULL == bucket->value) {
        return FAILED;
    }

    while (strcmp(key, bucket->key)) {
        if (NULL != bucket->next) {
            bucket = bucket->next;
        } else {
            break;
        }
    }

    if (NULL == bucket->value || strcmp(key, bucket->key)) {
        return FAILED;
    }

    memset(bucket, 0, sizeof(Bucket));
    ht->total --;
    return SUCCESS;
}

void hash_status(HashTable *ht) {
    printf("Total Size:\t\t%d\n", ht->total);
    printf("Current Size:\t\t%d\n", ht->size);
}

int hash_index(HashTable *ht, char *key) {
    return ELFHash(key, ht->size);
}

// ELF Hash Function
static unsigned int ELFHash(char *str, unsigned int length){
    unsigned int hash = 0;
    unsigned int x = 0;
    
    while (*str)
    {
        hash = (hash << 4) + (*str++);
        if ((x = hash & 0xF0000000L) != 0)
        {
            hash ^= (x >> 24);
            hash &= ~x;
        }
    }
    return (hash & 0x7FFFFFFF) % length;
}

int hash_keys(HashTable *ht, char *keys, int *len)
{
    Bucket *bucket;
    int index = 0;
    const char b = ' ';
    const char n = '\n';
    int kslen = 0;
    int klen = 0;
    char *ks = keys;

    if(ht->total >0)
    {

        for(index=0; index<ht->size; index++)
        {
            bucket = &ht->buckets[index];
            
            if (bucket->key == NULL) {
                continue;
            }
            
            klen = strlen(bucket->key);
        
            memcpy(ks, bucket->key, klen);
            ks+=klen;
            memcpy(ks, &b, 1);
            ks++;
            kslen = kslen+klen+1;

        }
        
        memcpy(ks, &n, 1);
        kslen = kslen+1;
    }
    

    *len = *len+kslen;
    return SUCCESS;
}

