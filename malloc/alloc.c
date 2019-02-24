/**
 * Malloc Lab
 * CS 241 - Spring 2019
 */
 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>

typedef struct _mem_block {
	struct _mem_block * prev;
	struct _mem_block * next;
	size_t bsize; //block size
} mem_block;

static void* HEAP_END_ADDR = NULL; //The very start addr of mem chunk
static void* HEAP_START_ADDR = NULL; //The very last bit of mem chunk
static mem_block* HEAD = NULL; //The first mem_block
static mem_block* TAIL = NULL; //The last mem_block
static size_t DATA_SIZE = sizeof(struct _mem_block);
static bool INITIALIZED = false;
static size_t ROUNDUP = 15;
static size_t MEM_ALIGN_SIZE = 16;
static mem_block* FORWARD = NULL;

mem_block* mem_get(void* ptr) {
	return (mem_block*) (((char*) ptr) - DATA_SIZE);
}

void* mem_out(mem_block* curr) {
	return (void*) (((char*) curr) + DATA_SIZE);
}

bool mem_check(mem_block* curr) {
	return (curr->bsize) & 1;
}

size_t mem_realsize(mem_block* curr) {
	return (curr->bsize) & ~1;
}

void mem_set(mem_block* curr) {
	curr->bsize |= 1;
}

void mem_unset(mem_block* curr) {
	curr->bsize &= ~1;
}

mem_block* mem_find(mem_block* input) {
	mem_block* curr;
	for(curr = input; curr != NULL; curr = curr->next) {
		if (!mem_check(curr)) {
			return curr;
		}
	}
	return NULL;
}

size_t mem_info(mem_block* curr) {
	return curr->bsize - DATA_SIZE;
}

/* 
 * Try to combine memory blocks
 * This will return a mem_block*
 * Check mem_block * ptr->asize to see if the block is large enough
 * B->B->p->q->r->B->B
*/
mem_block* mem_combine(mem_block* curr) {
	if (curr == TAIL && curr == HEAD) {
		return curr;
	} else if (curr == TAIL && curr != HEAD) {
		if (mem_check(curr->prev)) {
			return curr;
		} else {
			curr->prev->bsize += mem_realsize(curr);
			curr->prev->next = NULL;
			TAIL = curr->prev;
			memset(curr, 0, DATA_SIZE);
			return TAIL;
		}
	} else if (curr == HEAD && curr != TAIL) {
		if (mem_check(curr->next)) {
			return curr;
		} else {
			if (curr->next == TAIL) {
				curr->bsize += mem_realsize(curr->next);
				curr->next = NULL;
				memset(TAIL, 0, DATA_SIZE);
				TAIL = curr;
				return curr;
			} else {
				mem_block* temp = curr->next;
				curr->bsize += mem_realsize(curr->next);
				curr->next = curr->next->next;
				if (curr->next != NULL) {
					curr->next->prev = curr;
				}	
				memset(temp, 0, DATA_SIZE);
				return curr;
			}
		}
	} else { //curr != HEAD && curr != TAIL
		bool prev_aval = !mem_check(curr->prev);
		bool next_aval = !mem_check(curr->next);
		if (!prev_aval && !next_aval) {
			return curr;
		} else if (!prev_aval && next_aval) {
			if (curr->next == TAIL) {
				curr->bsize += mem_realsize(curr->next);
				curr->next = NULL;
				memset(TAIL, 0, DATA_SIZE);
				TAIL = curr;
				return curr;
			} else {
				mem_block* temp = curr->next;
				curr->bsize += mem_realsize(curr->next);
				curr->next = curr->next->next;
				if (curr->next != NULL) {
					curr->next->prev = curr;
				}
				memset(temp, 0, DATA_SIZE);
				return curr;
			}
		} else if (prev_aval && !next_aval) {
			curr->prev->next = curr->next;
			curr->next->prev = curr->prev;
			curr->prev->bsize += mem_realsize(curr);
			mem_block* temp = curr;
			curr = curr->prev;
			memset(temp, 0, DATA_SIZE);
			return curr;	
		} else { //prev_aval && next_aval
			curr->prev->next = curr->next->next;
			if (curr->prev->next != NULL) {
				curr->prev->next->prev = curr->prev;
			}
			curr->prev->bsize += mem_realsize(curr) + mem_realsize(curr->next);
			mem_block* temp = curr;
			curr = curr->prev;
			memset(temp->next, 0, DATA_SIZE);
			memset(temp, 0, DATA_SIZE);
			return curr;
		}
	}		
}


/* 
 * Try to frag one large data segment to desired size, dsize
 * return: the ready to use (void*) pointer
 * before return, the ready-to-use mem_block must be appropriately seperated
 * and chained with previous and next blocks
*/
void* mem_frag(mem_block* block_to_frag, size_t dsize) {
    size_t bsize = ((size_t) ((dsize + DATA_SIZE + ROUNDUP) / MEM_ALIGN_SIZE) * MEM_ALIGN_SIZE);
	
	mem_block* new_mem_block = (mem_block*) ((char*) block_to_frag + bsize);
	//FIRST memset all new block
	memset(new_mem_block, 0, mem_realsize(block_to_frag) - bsize);
	new_mem_block->bsize = mem_realsize(block_to_frag) - bsize;
	new_mem_block->prev = block_to_frag;
	new_mem_block->next = block_to_frag->next;
	if (block_to_frag->next != NULL) {
		block_to_frag->next->prev = new_mem_block;
	} else {
		TAIL = new_mem_block;
	}
	block_to_frag->bsize = bsize;
	mem_set(block_to_frag);
	mem_unset(new_mem_block);
	block_to_frag->next = new_mem_block;
	return mem_out(block_to_frag);
}

/*
 * Only constructs new chunk in the end
 * Result:
 * 1. Old Tail->next will be new chunk
 * 2. New Tail will be new chunk
 * 3. New Tail->prev will be Old Tail
 * 4. New Tail->next should be NULL
*/
void* mem_construct(size_t size) {
	if (!INITIALIZED) {
		INITIALIZED = true;
		size_t bsize =((size_t) ((size + DATA_SIZE + ROUNDUP) / MEM_ALIGN_SIZE) * MEM_ALIGN_SIZE);
		mem_block* curr = (mem_block*) sbrk(bsize);
		if (curr == (void*) -1) {
			return NULL;
		}
		//bsize + 1 for occupied
		curr->bsize = bsize;
		curr->prev = NULL;
		curr->next = NULL;
		mem_set(curr);
		HEAD = curr;
		TAIL = curr;
		HEAP_END_ADDR = ((char*) curr) + bsize;
		HEAP_START_ADDR = curr;
		return mem_out(curr);
	} else {
		size_t bsize = ((size_t) ((size + DATA_SIZE + ROUNDUP) / MEM_ALIGN_SIZE) * MEM_ALIGN_SIZE);
		mem_block* curr = (mem_block*) sbrk(bsize);
		if (curr == (void*) -1) {
			return NULL;
		}
		//bsize + 1 for occupied
		curr->bsize = bsize;
		curr->prev = TAIL;
		curr->prev->next = curr;
		mem_set(curr);
		TAIL = curr;
		curr->next = NULL;
		HEAP_END_ADDR += bsize;
		return mem_out(curr);
	}
}

void* mem_dispense(size_t size) {
	if (!INITIALIZED) {
		return mem_construct(size);
	}
	//CHECK current mem_block linked list
	mem_block* curr = NULL;
	size_t curr_bsize = 0;
	size_t bsize = ((size_t) ((size + DATA_SIZE + ROUNDUP) / MEM_ALIGN_SIZE) * MEM_ALIGN_SIZE);
	for (curr = FORWARD; curr != NULL; curr = curr->next) {
		curr_bsize = mem_realsize(curr);
		if (mem_check(curr)) {
			continue;
		}
		if (curr_bsize < bsize) {
			continue;
		}
		
		if (curr_bsize == bsize) { //PERFECT FIT
			mem_set(curr);
			FORWARD = (curr == FORWARD) ? mem_find(curr) : FORWARD;
			return mem_out(curr);
		}
		size_t remainder = curr_bsize - bsize;
		if (curr_bsize > bsize) {
			if (remainder % MEM_ALIGN_SIZE == 0) {
				mem_set(curr);
				FORWARD = (curr == FORWARD) ? mem_find(curr) : FORWARD; 
				return mem_frag(curr, size);
			} else {
				mem_set(curr);
				FORWARD = (curr == FORWARD) ? mem_find(curr) : FORWARD;	
				return mem_out(curr);
			}
		}
	}	
	return mem_construct(size);	
}

/**
 * Allocate space for array in memory
 *
 * Allocates a block of memory for an array of num elements, each of them size
 * bytes long, and initializes all its bits to zero. The effective result is
 * the allocation of an zero-initialized memory block of (num * size) bytes.
 *
 * @param num
 *    Number of elements to be allocated.
 * @param size
 *    Size of elements.
 *
 * @return
 *    A pointer to the memory block allocated by the function.
 *
 *    The type of this pointer is always void*, which can be cast to the
 *    desired type of data pointer in order to be dereferenceable.
 *
 *    If the function failed to allocate the requested block of memory, a
 *    NULL pointer is returned.
 *
 * @see http://www.cplusplus.com/reference/clibrary/cstdlib/calloc/
 */
void *calloc(size_t num, size_t size) {
	void* malloc_result = malloc(num * size);
	if (malloc_result != NULL) {
		memset(malloc_result, 0, num * size);
	}
    return malloc_result;
}

/**
 * Allocate memory block
 *
 * Allocates a block of size bytes of memory, returning a pointer to the
 * beginning of the block.  The content of the newly allocated block of
 * memory is not initialized, remaining with indeterminate values.
 *
 * @param size
 *    Size of the memory block, in bytes.
 *
 * @return
 *    On success, a pointer to the memory block allocated by the function.
 *
 *    The type of this pointer is always void*, which can be cast to the
 *    desired type of data pointer in order to be dereferenceable.
 *
 *    If the function failed to allocate the requested block of memory,
 *    a null pointer is returned.
 *
 * @see http://www.cplusplus.com/reference/clibrary/cstdlib/malloc/
 */
void *malloc(size_t size) {
	if (size == 0) {
		return NULL;
	}
    return mem_dispense(size);
}

/**
 * Deallocate space in memory
 *
 * A block of memory previously allocated using a call to malloc(),
 * calloc() or realloc() is deallocated, making it available again for
 * further allocations.
 *
 * Notice that this function leaves the value of ptr unchanged, hence
 * it still points to the same (now invalid) location, and not to the
 * null pointer.
 *
 * @param ptr
 *    Pointer to a memory block previously allocated with malloc(),
 *    calloc() or realloc() to be deallocated.  If a null pointer is
 *    passed as argument, no action occurs.
 */
void free(void *ptr) {
    if (ptr == NULL) {
		return;
	}
	mem_block* curr = mem_get(ptr);
	if (mem_check(curr)) {
		mem_unset(curr);
	}
	memset(ptr, 0, mem_info(curr));
	mem_block* comb = mem_combine(curr);
	if (FORWARD == NULL) {
		FORWARD = comb;
	} else {
		if (comb < FORWARD) {
			FORWARD = comb;
		}
	}
	mem_unset(comb);
	return;
}

/**
 * Reallocate memory block
 *
 * The size of the memory block pointed to by the ptr parameter is changed
 * to the size bytes, expanding or reducing the amount of memory available
 * in the block.
 *
 * The function may move the memory block to a new location, in which case
 * the new location is returned. The content of the memory block is preserved
 * up to the lesser of the new and old sizes, even if the block is moved. If
 * the new size is larger, the value of the newly allocated portion is
 * indeterminate.
 *
 * In case that ptr is NULL, the function behaves exactly as malloc, assigning
 * a new block of size bytes and returning a pointer to the beginning of it.
 *
 * In case that the size is 0, the memory previously allocated in ptr is
 * deallocated as if a call to free was made, and a NULL pointer is returned.
 *
 * @param ptr
 *    Pointer to a memory block previously allocated with malloc(), calloc()
 *    or realloc() to be reallocated.
 *
 *    If this is NULL, a new block is allocated and a pointer to it is
 *    returned by the function.
 *
 * @param size
 *    New size for the memory block, in bytes.
 *
 *    If it is 0 and ptr points to an existing block of memory, the memory
 *    block pointed by ptr is deallocated and a NULL pointer is returned.
 *
 * @return
 *    A pointer to the reallocated memory block, which may be either the
 *    same as the ptr argument or a new location.
 *
 *    The type of this pointer is void*, which can be cast to the desired
 *    type of data pointer in order to be dereferenceable.
 *
 *    If the function failed to allocate the requested block of memory,
 *    a NULL pointer is returned, and the memory block pointed to by
 *    argument ptr is left unchanged.
 *
 * @see http://www.cplusplus.com/reference/clibrary/cstdlib/realloc/
 */
void *realloc(void *ptr, size_t size) {
   	if (ptr == NULL) {
		return malloc(size);
	}
	if (size == 0) {
		free(ptr);
		return NULL;
	}
	mem_block* curr = mem_get(ptr);
	size_t asize = mem_info(curr);
	void* malloc_result = malloc(size);
	if (malloc_result == NULL) {
		return NULL;
	}
	size_t cpy_len = (asize > size) ? size : asize;
	memcpy(malloc_result, ptr, cpy_len);
	free(ptr);
	return malloc_result;
}
