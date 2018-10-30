#ifndef __HASH_MAP_H__
#define __HASH_MAP_H__

#include "link_list.h"

#define DEFAULT_INITIAL_CAPACITY 100000


class HashMap
{
	int size;   //大小
    int capacity; //容量
    int capacity_tmp;
    //float loadFactor;   //加载因子
    int (*hashCode)(const void *key);
    int (*equal)(const void *key1, const void *key2);
	//llist_node_op list_node_op;
    LLIST ** entryList;

	//void node_destroy(void *node);
	
	public:
	HashMap(int (*hashCode)(const void *key),int (*equal)(const void *key1, const void *key2), int capacity = DEFAULT_INITIAL_CAPACITY);
	~HashMap();
	int init(void);
	int insert(const void *  key, int key_size, const void * value, int value_size);
	void *find(const void *  key);
	int has_key(const void * key);
	int erase(const void * key);
	int clear(void);
	int get_size(void);
	//int print(llist_op *op);
};

void * hashmap_key_get(const void *entry);

#endif

