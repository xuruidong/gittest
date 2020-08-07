#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sched.h>
#include "x_ring.h"


static int atomic32_cmpset(volatile unsigned int *dst, unsigned int exp, unsigned int src)
{
	unsigned char res;

	asm volatile(
			"lock; "
			"cmpxchgl %[src], %[dst];"
			"sete %[res];"
			: [res] "=a" (res),     /* output */
			  [dst] "=m" (*dst)
			: [src] "r" (src),      /* input */
			  "a" (exp),
			  "m" (*dst)
			: "memory");            /* no-clobber list */
	return res;
}

static int is_power_of_2(unsigned long n)
{
	return (n != 0 && ((n & (n - 1)) == 0));
}

static unsigned long roundup_power_of_two(unsigned long n)
{
    if((n & (n-1)) == 0)
        return n;
    
    unsigned long maxulong = (unsigned long)((unsigned long)~0);
    unsigned long andv = ~(maxulong&(maxulong>>1));

    while((andv & n) == 0)
        andv = andv>>1;

    return andv<<1;
}

//#define X_RING_DEBUG(r) {printf("[%s:%d] ph=%d, pt=%d, ch=%d, ct=%d\n", __FUNCTION__, __LINE__, (r)->prod.head, (r)->prod.tail, (r)->cons.head, (r)->cons.tail);}
#define X_RING_DEBUG(r)

x_ring_t * x_ring_create(unsigned int count)
{
	int i = 0;
	x_ring_t * ringbuf = NULL;

	if (!is_power_of_2(count)) {
        count = roundup_power_of_two(count);
	}
	
	ringbuf = (x_ring_t *)malloc(sizeof(x_ring_t)+sizeof(x_ring_entry_t)*count);
	if (NULL == ringbuf){
		perror("ring_create()\n");
		return NULL;
	}

	ringbuf->prod.head = 0;
	ringbuf->prod.tail = 0;
	ringbuf->cons.head = 0;
	ringbuf->cons.tail = 0;
	ringbuf->size = count;
	ringbuf->mask = count-1;
	
	for(i=0; i<count; i++){
		ringbuf->ring[i].size = 512;
		ringbuf->ring[i].data = (char *)malloc(512);
		//printf("ring[%d] at %p, offset=%x\n", i, &(ringbuf->ring[i]), &(((x_ring_t *)NULL)->ring[i]));
		/*
		ringbuf->ring[i] = malloc(sizeof(ring_entry_t));
		if(NULL == ringbuf->ring[i]){
			perror("ring malloc\n");
			return NULL;
		}
		((ring_entry_t *)(ringbuf->ring[i]))->buf = (char *)malloc(LOG_BUFF_LEN);
		((ring_entry_t *)(ringbuf->ring[i]))->size = LOG_BUFF_LEN;
		*/
	}
	
	return ringbuf;
}

void x_ring_destroy(x_ring_t *ringbuf, x_ring_data_op *op)
{
	int i=0;
	for(i=0; i<ringbuf->size; i++){
		if (ringbuf->ring[i].data){
			if (op){
				op(ringbuf->ring[i].data);
			}
			/*
			if(((ring_entry_t *)(ringbuf->ring[i]))->data){
				free(((ring_entry_t *)(ringbuf->ring[i]))->data);
				((ring_entry_t *)(ringbuf->ring[i]))->size = 0;
			}
			*/
			free(ringbuf->ring[i].data);
			ringbuf->ring[i].data = NULL;
		}
	}
	
	free(ringbuf);
}

int x_ring_sp_enqueue(x_ring_t *ringbuf, const void *data, int size)
{
	uint32_t prod_head, cons_tail;
	uint32_t prod_next;

	prod_head = ringbuf->prod.head;
	cons_tail = ringbuf->cons.tail;
	prod_next = (prod_head+1)&ringbuf->mask;

	if (prod_next == cons_tail){
		//lock or sleep ???
		//...
		return -X_RING_RET_NOSPACE;
	}
	ringbuf->prod.head = prod_next;
	
	if (ringbuf->ring[prod_head].size < size){
		free(ringbuf->ring[prod_head].data);
		ringbuf->ring[prod_head].data = malloc(size);
		ringbuf->ring[prod_head].size = size;
	}
	memcpy(ringbuf->ring[prod_head].data, data, size);
	ringbuf->ring[prod_head].content_length = size;

	ringbuf->prod.tail = prod_next;
	X_RING_DEBUG(ringbuf);
	return X_RING_RET_SUCCESS;
}

int x_ring_data_copy(void *ring_data_buf, const void *data, size_t size, void *arg)
{
	(void)arg;
	memcpy(ring_data_buf, data, size);
	return 0;
}

int x_ring_sp_enqueue2(x_ring_t *ringbuf, const void *data, int size, int (*copy) (void *, const void *, size_t, void *), void *arg)
{
	uint32_t prod_head, cons_tail;
	uint32_t prod_next;

	prod_head = ringbuf->prod.head;
	cons_tail = ringbuf->cons.tail;
	prod_next = (prod_head+1)&ringbuf->mask;

	if (prod_next == cons_tail){
		//lock or sleep ???
		//...
		return -X_RING_RET_NOSPACE;
	}
	ringbuf->prod.head = prod_next;
	
	if (ringbuf->ring[prod_head].size < size){
		free(ringbuf->ring[prod_head].data);
		ringbuf->ring[prod_head].data = malloc(size);
		ringbuf->ring[prod_head].size = size;
	}
	
	//memcpy(ringbuf->ring[prod_head].data, data, size);
	copy(ringbuf->ring[prod_head].data, data, size, arg);
	ringbuf->ring[prod_head].content_length = size;

	ringbuf->prod.tail = prod_next;

	return X_RING_RET_SUCCESS;
}


int x_ring_sp_enqueue3(x_ring_t *ringbuf, const void *data, int size)
{
	x_ring_sp_enqueue2(ringbuf, data, size, x_ring_data_copy, NULL);
	return 0;
}


int x_ring_mp_enqueue(x_ring_t *ringbuf, const void *data, int size)
{
	int res = 0;
	uint32_t prod_head = 0;
	uint32_t prod_next = 0;
	unsigned int i = 0;

	do{
		prod_head = ringbuf->prod.head;
#if 0
		next_prod = prod_index+1;
		if (next_prod >= ringbuf->ring_size){
			next_prod = 0;
		}
#else
		prod_next = (prod_head+1)&ringbuf->mask;
#endif

		if (prod_next == ringbuf->cons.tail){
			//lock or sleep ???
			//...
			return -X_RING_RET_NOSPACE;
		}
		//printf("[%s:%d]remember\n", __FILE__, __LINE__);

		res = atomic32_cmpset(&(ringbuf->prod.head), prod_head, prod_next);
	}while(res != 1);

	if (ringbuf->ring[prod_head].size < size){
		free(ringbuf->ring[prod_head].data);
		ringbuf->ring[prod_head].data = malloc(size);
		ringbuf->ring[prod_head].size = size;
	}
	memcpy(ringbuf->ring[prod_head].data, data, size);
	ringbuf->ring[prod_head].content_length = size;

	// update productor.tail
	while(ringbuf->prod.tail != prod_head){
		i++;
		if (i>= X_RING_PAUSE_REP_COUNT){
			i = 0;
			printf("enqueue sched_yield();\n");
			sched_yield();
		}
	}
	ringbuf->prod.tail = prod_next;

	return X_RING_RET_SUCCESS;
}

int x_ring_sc_dequeue(x_ring_t *r, x_ring_entry_t *out)
{
	uint32_t cons_head, prod_tail;
	uint32_t cons_next;
	//unsigned i;
	//uint32_t mask = r->prod.mask;

	cons_head = r->cons.head;
	prod_tail = r->prod.tail;
	X_RING_DEBUG(r);
	if ((cons_head==prod_tail)){
		return -X_RING_RET_NOENT;
	}

	cons_next = (cons_head + 1)&r->mask;
	
	r->cons.head = cons_next;

	/* copy in table */
	if ( out->size < r->ring[cons_head].content_length ){
		free(out->data);
		out->data = malloc(r->ring[cons_head].size);
	}
	memcpy(out->data, r->ring[cons_head].data, r->ring[cons_head].content_length);
	out->content_length = r->ring[cons_head].content_length;

	r->cons.tail = cons_next;
	return X_RING_RET_SUCCESS;
}

int x_ring_mc_dequeue(x_ring_t *ringbuf, x_ring_entry_t *out)
{
	int res = 0;
	uint32_t cons_head = 0;
	uint32_t cons_next = 0;
	uint32_t prod_tail = 0;
	unsigned int i = 0;
	x_ring_entry_t *p = NULL;

	do{
		cons_head = ringbuf->cons.head;
		prod_tail = ringbuf->prod.tail;
		//printf("[%s:%d] cons=%d, prod=%d\n", __FUNCTION__, __LINE__, curr_cons, curr_prod);
		if ((cons_head==prod_tail)){
			return -X_RING_RET_NOENT;
		}
#if 0
		next_cons = curr_cons + 1;
		if(next_cons >= ringbuf->ring_size){
			next_cons = 0;
		}
#else
		cons_next = (cons_head + 1)&ringbuf->mask;
#endif
		
		res = atomic32_cmpset(&(ringbuf->cons.head), cons_head, cons_next);
	}while (res != 1);
	//printf("[%s:%d] curr_cons=%d\n", __FUNCTION__, __LINE__, curr_cons);
	p = &(ringbuf->ring[cons_head]);
	if ( out->size < p->content_length ){
		free(out->data);
		out->data = malloc(p->size);
	}
	memcpy(out->data, p->data, p->content_length);
	out->content_length = p->content_length;

	while(ringbuf->cons.tail != cons_head){
		i++;
		if (i>= X_RING_PAUSE_REP_COUNT){
			i = 0;
			printf("dequeue sched_yield();\n");
			sched_yield();
		}
	}
	ringbuf->cons.tail = cons_next;
	return X_RING_RET_SUCCESS;
}


void rte_ring_dump(FILE *f, const x_ring_t *r)
{

}

int x_ring_print(x_ring_t *r)
{
	int i = 0;
	int ring_size = r->size;
	int ret = 0;
	printf("ring_size=%d\n", ring_size);
	x_ring_entry_t ring_entry;
	ring_entry.size = 1024;
	ring_entry.content_length = 0;
	ring_entry.data = malloc(ring_entry.size);
	for(i=0; i<ring_size; i++){
		ret = x_ring_mc_dequeue(r, &ring_entry);
		if(ret < 0){
			break;
		}
		printf("ring %d, %s\n",i, ring_entry.data);
	}
	free(ring_entry.data);
	return i;
}

int x_ring_full(const x_ring_t *r)
{
	uint32_t prod_tail = r->prod.tail;
	uint32_t cons_tail = r->cons.tail;
	return ((cons_tail - prod_tail - 1) & r->mask) == 0;
}

int x_ring_empty(const x_ring_t *r)
{
	uint32_t prod_tail = r->prod.tail;
	uint32_t cons_tail = r->cons.tail;
	return !!(cons_tail == prod_tail);
}

unsigned x_ring_count(const x_ring_t *r)
{
	uint32_t prod_tail = r->prod.tail;
	uint32_t cons_tail = r->cons.tail;
	return (prod_tail - cons_tail) & r->mask;
}

unsigned x_ring_free_count(const x_ring_t *r)
{
	uint32_t prod_tail = r->prod.tail;
	uint32_t cons_tail = r->cons.tail;
	return (cons_tail - prod_tail - 1) & r->mask;
}

// ------  ringbuf end  ------------
