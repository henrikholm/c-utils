/*
 * array.h
 *
 *  Created on: Dec 9, 2013
 *      Author: holm
 */

#ifndef ARRAY_H_
#define ARRAY_H_

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "../logging/logging.h"

#define DEFAULT_CHUNK_SIZE 1000
#define DEFAULT_NR_CHUNKS 1



typedef void* chunkarray;

typedef int (*chunkarray_equal)(void *, void *);
typedef void (*chunkarray_free_item)(void *);


typedef struct {
	chunkarray (*create)(chunkarray_equal eq, uint32_t item_sz, uint32_t chunk_size);
	void (*free)(chunkarray c);
	int (*size)(chunkarray c);
	/*
	 void (*free_items)(chunkarray, chunkarray_free_item);
	 int (*is_empty)(chunkarray);
	 int (*contains)(chunkarray, void *);
	 void *(*get_first)(chunkarray);
	 void *(*get_last)(chunkarray);
	 */
	void *(*get)(chunkarray c, uint32_t index);
	/*
	 int (*add_first)(chunkarray, void *);
	 */
	void* (*add_last)(chunkarray c, void* item);
	void* (*set)(chunkarray c, uint32_t index, void* item);
/*
 int (*remove)(chunkarray, void *);
 void *(*remove_first)(chunkarray);
 void *(*remove_last)(chunkarray);
 */
} chunkarray_ns;

extern chunkarray_ns const chunkarray_methods;





#endif /* ARRAY_H_ */
