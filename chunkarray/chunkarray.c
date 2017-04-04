
#include "chunkarray.h"

typedef struct _chunkarray* chunkarray_t;

typedef struct {
	uint32_t size;
	uint8_t* data;

} chunk;

struct _chunkarray {
	uint32_t size;
	uint32_t chunk_sz;
	uint32_t nr_chunks;
	uint32_t item_sz;
	chunk* chunks;
	chunkarray_equal equal;
};


int _chunkarray_equal_pointers(void *item1, void *item2) {
  return item1 == item2;
}


int __add_chunks(chunkarray_t ca){
	chunk* new_chunks = malloc(sizeof(chunk) * ca->nr_chunks*2);
	if (!new_chunks) {
		free(new_chunks);
		return 0;
	}

	logging_methods.info("ChunkArray: Adding chunks. Was :%d chunks of size %d (%d items), will be %d chunks\n", ca->nr_chunks, ca->item_sz * ca->chunk_sz, ca->chunk_sz, ca->nr_chunks*2 );

	memcpy(new_chunks, ca->chunks, sizeof(chunk) * ca->nr_chunks);

	free(ca->chunks);

	ca->chunks = new_chunks;
	for(uint32_t i = ca->nr_chunks; i < (ca->nr_chunks*2); i++){
		ca->chunks[i].size = 2;
		ca->chunks[i].data = malloc(ca->item_sz * ca->chunk_sz);
	}

	ca->nr_chunks *= 2;

	return 1;
}



chunkarray _chunkarray_create(chunkarray_equal eq, uint32_t item_sz, uint32_t chunk_size) {
	chunkarray_t ca = malloc(sizeof(struct _chunkarray));
	if (!ca){
		return NULL;
	}

	if(item_sz < 1){
		return NULL;
	}

	ca->size = 0;
	ca->chunk_sz = (chunk_size > 0) ? chunk_size : DEFAULT_CHUNK_SIZE;
	ca->nr_chunks = DEFAULT_NR_CHUNKS;
	ca->item_sz = item_sz;
	ca->equal = (eq) ? eq : _chunkarray_equal_pointers;

	// Init chunk holder
	ca->chunks = malloc(sizeof(chunk) * ca->nr_chunks);
	if (!ca->chunks) {
		free(ca);
		return NULL;
	}

	//Init chunks, each chunk holds "chunk_sz" items
	for(uint32_t i = 0; i < ca->nr_chunks; i++){
		ca->chunks[i].size = 1;
		ca->chunks[i].data = malloc(ca->item_sz * ca->chunk_sz);
	}

	return (chunkarray)ca;
}

void _chunkarray_free(chunkarray c){
	if(!c){
		return;
	}
	chunkarray_t ca = (chunkarray_t)c;
	for(uint32_t i = 0; i < ca->nr_chunks; i++){
		free(ca->chunks[i].data);
	}
	free(ca->chunks);
	free(ca);

	return;
}

void* _chunkarray_set(chunkarray c, uint32_t index, void* item){
	if(!c){
		return NULL;
	}
	chunkarray_t ca = (chunkarray_t)c;

	if(index > ca->size){
		logging_methods.info("ChunkArray: Tried to add item for index %d larger than size %d\n", index, ca->size);
		return NULL;
	}

	uint32_t chunk_index = index/ca->chunk_sz;
	uint32_t item_index = index%ca->chunk_sz;

	if(chunk_index >= ca->nr_chunks){
		if(!__add_chunks(ca)){
			return NULL;
		}
	}

	uint32_t offset = item_index*ca->item_sz;

	// If we added this one last in the list
	if(index == ca->size){
		ca->size++;
	}

	chunk ch = ca->chunks[chunk_index];

	return memcpy(&ch.data[offset], item, ca->item_sz);
}

void* _chunkarray_get(chunkarray c, uint32_t index){
	if(!c){
		return NULL;
	}
	chunkarray_t ca = (chunkarray_t)c;

	if(index >= ca->size){
		return NULL;
	}

	uint32_t chunk_index = index/ca->chunk_sz;
	uint32_t item_index = index%ca->chunk_sz;
	uint32_t offset = item_index*ca->item_sz;

	return (void*)&ca->chunks[chunk_index].data[offset];

}

void* _chunkarray_add_last(chunkarray c, void* item){
	if(!c){
		return NULL;
	}
	chunkarray_t ca = (chunkarray_t)c;
	uint32_t index = ca->size;

	return _chunkarray_set(c, index, item);
}

int _chunkarray_size(chunkarray c){
	if(!c){
		return -1;
	}
	chunkarray_t ca = (chunkarray_t)c;
	return ca->size;
}

chunkarray_ns const chunkarray_methods = {
		.create = &_chunkarray_create,
		.free = &_chunkarray_free,
		.size = &_chunkarray_size,

/*
 .free_items = &_arraylist_free_items,
 .size = &_arraylist_size,
 .is_empty = &_arraylist_is_empty,
 .contains = &_arraylist_contains,
 .get_first = &_arraylist_get_first,
 .get_last = &_arraylist_get_last,
 */
		.get = &_chunkarray_get,
/*
 .to_array = &_arraylist_to_array,
 .add_first = &_arraylist_add_first,
 */
		.add_last = &_chunkarray_add_last, .set = &_chunkarray_set
/*
 .remove = &_arraylist_remove,
 .remove_first = &_arraylist_remove_first,
 .remove_last = &_arraylist_remove_last,
 */
};

