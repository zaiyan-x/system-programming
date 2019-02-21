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
	size_t asize; //Asked or Requested size
	size_t bsize; //Real b_lock size
	bool occupied;
} mem_block;

static void* HEAP_END_ADDR = NULL; //The very start addr of mem chunk
static void* HEAP_START_ADDR = NULL; //The very last bit of mem chunk
static mem_block* HEAD = NULL; //The first mem_block
static mem_block* TAIL = NULL; //The last mem_block
static size_t DATA_SIZE = sizeof(mem_block);
static bool INITIALIZED = false;
static size_t ROUNDUP = 15;
static size_t MEM_ALIGN_SIZE = 16;

/* 
 * Try to combine memory blocks
 * This will return a mem_block*
 * Check mem_block * ptr->asize to see if the block is large enough
 * B->B->p->q->r->B->B
*/
mem_block* mem_combine(mem_block* curr) {
	if (curr == NULL || curr->occupied) { //curr is not freed, directly return
		return curr;
	}
	// mem_block attributes
	mem_block* curr_block = curr;
	size_t total_user_size = curr->asize;
	size_t total_block_size = curr->bsize;
	
	if (curr->prev == NULL && curr->next == NULL) {
		return curr;
	} else if (curr->prev != NULL && curr->next != NULL) {
		// Maximal case
		// Combine all three chunks
		if (!curr->prev->occupied && !curr->next->occupied) {
			if (curr->next->next == NULL) {
				curr_block->prev->next = NULL;
			} else {
				curr_block->prev->next = curr_block->next->next;
				curr_block->next->next->prev = curr_block->prev;
			}
			total_user_size += DATA_SIZE + DATA_SIZE + curr->prev->asize + curr->next->asize;
			total_block_size +=	curr->prev->bsize + curr->next->bsize;
			curr_block = curr_block->prev;
			memset(curr->next, 0, DATA_SIZE);
			memset(curr, 0, DATA_SIZE);
			curr_block->asize = total_user_size;
			curr_block->bsize = total_block_size;
			return curr_block;
		// Combine none
		} else if (curr->prev->occupied && curr->next->occupied) {
			return curr_block;
		// Combine NEXT
		} else if (curr->prev->occupied && !curr->next->occupied) {
			if (curr->next->next == NULL) {
				curr_block->next = NULL;
			} else {
				curr_block->next = curr_block->next->next;
				curr_block->next->next->prev = curr_block;
			}
			total_user_size += DATA_SIZE + curr->next->asize;
			total_block_size += curr->next->bsize;
			memset(curr->next, 0, DATA_SIZE);
			curr_block->asize = total_user_size;
			curr_block->bsize = total_block_size;
			return curr_block;
		// Combine PREV
		} else {
			total_user_size += DATA_SIZE + curr->prev->asize;
			total_block_size += curr->prev->bsize;
			curr_block = curr_block->prev;
			memset(curr, 0, DATA_SIZE);
			curr_block->asize = total_user_size;
			curr_block->bsize = total_block_size;
			return curr_block;
		}
	} else if (curr->next == NULL && curr->prev != NULL) {
		if (curr->prev->occupied) {
			return curr_block;
		} else {
			curr_block->prev->next = NULL;
			total_user_size += DATA_SIZE + curr->prev->asize;
			total_block_size += curr_block->prev->bsize;
			curr_block = curr_block->prev;
			memset(curr, 0, DATA_SIZE);
			curr_block->asize = total_user_size;
			curr_block->bsize = total_block_size;
			return curr_block;
		}
	} else { //curr->next != NULL && curr->prev == NULL
		if (curr->next->occupied) {
			return curr_block;
		} else {
			if (curr_block->next->next == NULL) {
				curr_block->next = NULL;
			} else {
				curr_block->next = curr_block->next->next;
				curr_block->next->next->prev = curr_block;
			}
			total_user_size += DATA_SIZE + curr->next->asize;
			total_block_size += curr->next->bsize;
			memset(curr->next, 0, DATA_SIZE);
			curr_block->asize = total_user_size;
			curr_block->bsize = total_block_size;
			return curr_block;	
		}	
	}
}


/* 
 * Try to frag one large data segment to desired size, dsize
 * return: the ready to use mem_block *
 * before return, the ready-to-use mem_block must be appropriately seperated
 * and chained with previous and next blocks
*/
mem_block* mem_frag(mem_block* block_to_frag, size_t dsize) {


}

void* mem_dispense(size_t size) {
	if (!INITIALIZED) {
		size_t bsize =((size_t) ((size + DATA_SIZE + ROUNDUP) / MEM_ALIGN_SIZE) * MEM_ALIGN_SIZE);
		void* curr = sbrk(bsize);
		((mem_block*) curr)->asize = size;
		((mem_block*) curr)->bsize = bsize;
		((mem_block*) curr)->prev = NULL;
		((mem_block*) curr)->next = NULL;
		((mem_block*) curr)->occupied = true;
		HEAD = (mem_block*) curr;
		TAIL = (mem_block*) curr;
		HEAP_END_ADDR = curr + bsize;
		HEAP_START_ADDR = curr;
		return curr + DATA_SIZE;
	}
	//CHECK current mem_block linked list
	mem_block* curr;
	bool found = false;
	for (curr = HEAD; curr != NULL; curr = curr->next) {
		if (curr->occupied) {
			continue;
		}
		if (curr->asize < size) {
			continue;
		}
		if (curr->asize == size) { //PERFECT FIT
			return curr + DATA_SIZE;
		}
		if (curr->asize > size) {
			return mem_frag(curr, size) + DATA_SIZE;
		}
	}
	return NULL;	
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
    // implement calloc!
    return NULL;
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
    void* curr_block = mem_dispense(size);
    return curr_block;
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
    // implement free!
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
    // implement realloc!
    return NULL;
}
