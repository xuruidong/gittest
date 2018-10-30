#include <stdlib.h>
#include <string.h>
#include "hash_map.h"

typedef struct entry
{
    void * key;
    void * value;
} Entry;

void * hashmap_key_get(const void *entry)
{
	return (((Entry *)entry)->key);
}


static void node_destroy(void *node)
{
	Entry * n = (Entry *)node;
	if(n->key)
		free(n->key);
	if (n->value)
		free(n->value);
}


HashMap :: HashMap(int (*hashCode)(const void *key),int (*equal)(const void *key1, const void *key2), int capacity)
{
	int i = 0;
	this-> size = 0;
	this->capacity = 0;
	this->hashCode = hashCode;
	this->equal = equal;
	this->capacity_tmp = capacity;
	//this->list_node_op = node_op;
	this ->entryList = NULL;
	for (i=0; i< this->capacity; i++){
		this->entryList[i] = llist_creat(sizeof(Entry));
	}
}

HashMap :: ~HashMap()
{
	int i = 0;
	if (this ->entryList){
		for (i=0; i< this->capacity; i++){
			llist_destroy(this->entryList[i], node_destroy);
		}
		free(this->entryList);
	}
}

int HashMap :: init()
{	
	int i;
	this->capacity = this->capacity_tmp;
	this ->entryList = (LLIST **)malloc(sizeof(LLIST *) * this->capacity);
	for (i=0; i< this->capacity; i++){
		this->entryList[i] = llist_creat(sizeof(Entry));
		if (this->entryList[i] == NULL)
			return -1;
	}

	return 0;
}


int HashMap :: insert(const void * key, int key_size, const void * value, int value_size)
{
	if (key ==NULL || value == NULL)
		return -1;
	
	int hash_code = this->hashCode(key);
	int entry_index = hash_code%this->capacity;
	if (entry_index < 0){
		entry_index += this->capacity;
	}
	
	//printf ("insert -list num=%d\n", llist_getnum(this->entryList[entry_index]));
	Entry * e = (Entry *)llist_find(this->entryList[entry_index], key, this->equal);
	if (e == NULL){
		/*
		Entry * entry = (Entry*) malloc(sizeof(Entry));
		entry->key = key;
		entry->value = value;
		*/
		Entry entry;
		entry.key = malloc(key_size);
		memcpy(entry.key, key, key_size);
		if (value){
			entry.value = malloc(value_size);
			memcpy(entry.value, value, value_size);
		}
		if (llist_insert(this->entryList[entry_index], &entry, LLIST_BACKWARD) != 0)
			return -1;
		this->size ++;
		//printf("[%s:%d]\n", __FILE__, __LINE__);
	}
	else{
		free(e->value);
		if (value){
			e->value = malloc(value_size);
			memcpy(e->value, value, value_size);
		}
		else{
			e->value = NULL;
		}
		//printf("[%s:%d]\n", __FILE__, __LINE__);
	}
	//printf ("insert2 -list num=%d\n", llist_getnum(this->entryList[entry_index]));
	return 0;
}

void * HashMap :: find(const void * key)
{
	int hash_code = this->hashCode(key);
	int entry_index = hash_code%this->capacity;
	if (entry_index < 0){
		entry_index += this->capacity;
	}
	Entry * e = (Entry *)llist_find(this->entryList[entry_index], key, this->equal);
	if (e != NULL){
		return e->value;
	}
	return NULL;
}

int HashMap :: has_key(const void *key)
{
	int hash_code = this->hashCode(key);
	int entry_index = hash_code%this->capacity;
	if (entry_index < 0){
		entry_index += this->capacity;
	}
	Entry * e = (Entry *)llist_find(this->entryList[entry_index], key, this->equal);
	if (e != NULL){
		return 1;
	}
	return 0;
}

int HashMap :: erase(const void *key)
{
	int hash_code = this->hashCode(key);
	int entry_index = hash_code%this->capacity;
	int ret = 0;
	if (entry_index < 0){
		entry_index += this->capacity;
	}
	ret = llist_delet(this->entryList[entry_index], key, this->equal, node_destroy);
	if (ret == 0){
		size --;
	}
	return ret;
}

int HashMap :: clear()
{
	int i = 0;
	int n = 0;
	for (i=0; i< this->capacity; i++){
		n = llist_clear(this->entryList[i], node_destroy);
		size -= n;
	}
	return 0;
}

int HashMap :: get_size()
{
	return this->size;
}
/*
int HashMap :: print(llist_op *op)
{
	int i=0;
	for (i=0; i<this->capacity; i++){
		llist_travel(this->entryList[i], op);
	}
	return 0;
}
*/


