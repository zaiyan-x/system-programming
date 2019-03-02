/**
 * Malloc Lab
 * CS 241 - Spring 2019
 */
 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>

typedef struct _header {
	struct _header * prev;
	struct _header * next;
	size_t bsize; //block size
} header;

typedef struct _footer {
	size_t bsize;
} footer;
#define K 1024
static size_t HEADER_SIZE = sizeof(struct _header);
static size_t DATA_SIZE = sizeof(struct _header) + sizeof(struct _footer);
static size_t FOOTER_SIZE = sizeof(struct _footer);
static size_t ROUNDUP = 7;
static size_t MEM_ALIGN_SIZE = 8;
static header* FREE_HEAD = NULL;
static void* HEAP_HEAD = NULL;
static void* HEAP_TAIL = NULL;
static size_t FRAG_LIMIT = 48;
bool mem_header_check(header* curr) {
	return (curr->bsize) & 1;
}

bool mem_footer_check(footer* curr) {
	return (curr->bsize) & 1;
}

size_t mem_footer_realsize(footer* curr) {
	return (curr->bsize) & ~1;
}

size_t mem_header_realsize(header* curr) {
	return (curr->bsize) & ~1;
}

size_t mem_header_usersize(header* curr) {
	return mem_header_realsize(curr) - DATA_SIZE;
}

header* mem_get_header_from_user(void* ptr) {
	return (header*) (((char*) ptr) - HEADER_SIZE);
}

footer* mem_get_footer_from_header(header* curr) {
	return (footer*) (((char*) curr) + mem_header_realsize(curr) - FOOTER_SIZE);
}

header* mem_get_header_from_footer(footer* curr) {
	return (header*) (((char*) curr) + FOOTER_SIZE - mem_footer_realsize(curr));  
}

footer* mem_get_footer_from_end(void* ptr) {
	return (footer*) ((char*) ptr - FOOTER_SIZE);
}

header* mem_get_end_from_header(header* curr) {
	return (header*) (((char*) curr) + mem_header_realsize(curr));
}

void* mem_get_user_from_header(header* curr) {
	return (void*) (((char*) curr) + HEADER_SIZE);
}
/*
 * mem_set sets two size_t in both header and footer
 * to indicate it is occupied
*/
void mem_set(header* curr_header) {
	curr_header->bsize |= 1;
	footer* curr_footer = mem_get_footer_from_header(curr_header);
	curr_footer->bsize |= 1;
	return;
}

void mem_unset(header* curr_header) {
	curr_header->bsize &= ~1;
	footer* curr_footer = mem_get_footer_from_header(curr_header);
	curr_footer->bsize &= ~1;
	return;
}

void mem_confine(header* curr_header, size_t bsize) {
	curr_header->bsize = bsize;
	footer* curr_footer = (footer*) (((char*) curr_header) + bsize - FOOTER_SIZE);
	curr_footer->bsize = bsize;
	return;
}

size_t mem_plan(size_t asize) {
	return ((size_t) ((asize + DATA_SIZE + ROUNDUP) / MEM_ALIGN_SIZE) * MEM_ALIGN_SIZE);
}

bool mem_check_TAIL(header* curr) {
	void* curr_end = (void*) mem_get_end_from_header(curr);
	if (curr_end == HEAP_TAIL) {
		return true;
	} else if (curr_end > HEAP_TAIL) {
		fprintf(stderr, "WHAT THE FUCK IS GOING ON???");
		return true;
	} else {
		return false;
	}
}

bool mem_check_HEAD(header* curr) {
	void* curr_start = (void*) curr;
	if (curr_start == HEAP_HEAD) {
		return true;
	} else if (curr_start > HEAP_HEAD) {
		return false;
	} else {
		fprintf(stderr, "WHAT THE FUCK IS GOING ON???");
		return true;
	}
}
footer* mem_get_prev_footer_from_header(header* curr) {
	if (mem_check_HEAD(curr)) {
		return NULL;
	}
	footer* prev_footer = (footer*) (((char*) curr) - FOOTER_SIZE);
	if (mem_footer_check(prev_footer)) {
		return NULL;
	} else {
		return prev_footer;
	}
}
header* mem_get_next_header_from_header(header* curr) {
	if (mem_check_TAIL(curr)) {
		return NULL;
	}
	header* next_header = (header*) (((char*) curr) + mem_header_realsize(curr));
	if (mem_header_check(next_header)) {
		return NULL;
	} else {
		return next_header;
	}
}
void mem_unchain(header* curr_header) {
	if (curr_header->prev == NULL && curr_header->next == NULL) {
		FREE_HEAD = NULL;
		curr_header->prev = NULL;
		curr_header->next = NULL;
		return;
	}
	if (curr_header->prev != NULL) {
		curr_header->prev->next = curr_header->next;
	} else {
		FREE_HEAD = curr_header->next;
		if (curr_header->next != NULL) {
			curr_header->next->prev = NULL;
		}
	}
	if (curr_header->next != NULL) {
		curr_header->next->prev = curr_header->prev;
	} else {
		if (curr_header->prev != NULL) {
			curr_header->prev->next = NULL;
		}
	}
	curr_header->prev = NULL;
	curr_header->next = NULL;
	return;
}

header* mem_combine_prev(header* curr_header, header* prev_header) {
	size_t comb_bsize = mem_header_realsize(prev_header) + mem_header_realsize(curr_header);
	footer* curr_footer = mem_get_footer_from_header(curr_header);
	curr_header->next = NULL;
	curr_header->prev = NULL;
	memset(curr_header, 0, HEADER_SIZE);
	memset(curr_footer, 0, FOOTER_SIZE);
	mem_confine(prev_header, comb_bsize);
	mem_unset(prev_header);
	return prev_header;
}

header* mem_combine_next(header* curr_header, header* next_header) {
	//FIRST FIX FREE-LIST
	mem_unchain(next_header);
	if (FREE_HEAD == NULL) {
		FREE_HEAD = curr_header;
		curr_header->prev = NULL;
		curr_header->next = NULL;
	} else {
		FREE_HEAD->prev = curr_header;
		curr_header->next = FREE_HEAD;
		curr_header->prev = NULL;
		FREE_HEAD = curr_header;
	}
	size_t comb_bsize = mem_header_realsize(curr_header) + mem_header_realsize(next_header);
	footer* next_footer = mem_get_footer_from_header(next_header);
	memset(next_header, 0, HEADER_SIZE);
	memset(next_footer, 0, FOOTER_SIZE);
	mem_confine(curr_header, comb_bsize);
	mem_unset(curr_header);
	return curr_header;
}

header* mem_combine_both(header* curr_header, header* prev_header, header* next_header) {
	//FIRST FIX FREE-LIST
	mem_unchain(next_header);
	size_t comb_bsize = mem_header_realsize(curr_header) + mem_header_realsize(prev_header) + mem_header_realsize(next_header);
		
	footer* prev_footer = mem_get_footer_from_header(prev_header);
	footer* curr_footer = mem_get_footer_from_header(curr_header);
	footer* next_footer = mem_get_footer_from_header(next_header);
	memset(prev_header, 0, HEADER_SIZE);
	memset(prev_footer, 0, FOOTER_SIZE);
	memset(curr_header, 0, HEADER_SIZE);
	memset(curr_footer, 0, FOOTER_SIZE);
	memset(next_header, 0, HEADER_SIZE);
	memset(next_footer, 0, FOOTER_SIZE);
	mem_confine(prev_header, comb_bsize);
	mem_unset(prev_header);
	return prev_header;
}


header* mem_combine_none(header* curr_header) {
	if (FREE_HEAD == NULL) {
		FREE_HEAD = curr_header;
		curr_header->prev = NULL;
		curr_header->next = NULL;
		return curr_header;
	}
	curr_header->prev = NULL;
	curr_header->next = FREE_HEAD;
	FREE_HEAD->prev = curr_header;
	FREE_HEAD = curr_header;
	mem_unset(curr_header);
	return curr_header;
}	


/* 
 * Try to combine memory blocks
 * This will return a header*
 * There will be four subcases
 * 1. merge none
 * 2. merge both
 * 3. merge prev
 * 4. merge next
*/
header* mem_combine(header* curr) {
	footer* prev_footer = mem_get_prev_footer_from_header(curr);
	header* next_header = mem_get_next_header_from_header(curr);
	if (prev_footer == NULL && next_header == NULL) {
		return mem_combine_none(curr);
	} else if (prev_footer != NULL && next_header == NULL) {
		header* prev_header = mem_get_header_from_footer(prev_footer);
		return mem_combine_prev(curr, prev_header);
	} else if (prev_footer == NULL && next_header != NULL) {
		return mem_combine_next(curr, next_header);
	} else { //combine both
		header* prev_header = mem_get_header_from_footer(prev_footer);
		return mem_combine_both(curr, prev_header, next_header);
	}	
}

/* 
 * Try to frag one large, currenetly IN USE, memory chunk to desired size, dsize
 * return: the ready to use (void*) pointer
 * before return, the ready-to-use mem_block must be appropriately seperated
 * ,and the LATTER block FREED and chained with previous and next blocks
 * @ dsize is desired block size need NOT to be converted
*/
void* mem_frag_realloc(header* curr_header, size_t dsize) {
	size_t curr_bsize = mem_header_realsize(curr_header);
	size_t new_bsize = curr_bsize - dsize;
	mem_confine(curr_header, dsize);
	mem_set(curr_header);
	header* new_header = mem_get_end_from_header(curr_header);
	mem_confine(new_header, new_bsize);
	free(mem_get_user_from_header(new_header));
	return mem_get_user_from_header(curr_header);
}

/*
 * Try to frag a memory chunk from FREE_LIST
 * return: the ready to use (void*) pointer
 * before return, the ready-to-use mem_block must be appropriatedly seperated
 * and REMAINED chained with next and previous blocks
 * @ dsize is desired block size need NOT to be converted
*/
void* mem_frag_malloc(header* curr_header, size_t dsize) {
	size_t curr_bsize = mem_header_realsize(curr_header);
	size_t modified_bsize = curr_bsize - dsize;
	mem_confine(curr_header, modified_bsize);
	mem_unset(curr_header);
	header* new_header = mem_get_end_from_header(curr_header);
	mem_confine(new_header, dsize);
	mem_set(new_header);
	new_header->prev = NULL;
	new_header->next = NULL;
	return mem_get_user_from_header(new_header);
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
	size_t bsize = mem_plan(size);
 
	header* curr = (header*) sbrk(bsize);
	if (curr == (void*) -1) {
		return NULL;
	}
	if (HEAP_HEAD == NULL && HEAP_TAIL == NULL) {
		HEAP_HEAD = (void*) curr;
		HEAP_TAIL = (void*) ((char*) curr + bsize);
	} else {
		HEAP_TAIL = (void*) ((char*) curr + bsize);
	}
	curr->prev = NULL;
	curr->next = NULL;
	mem_confine(curr, bsize);
	mem_set(curr);	
	return mem_get_user_from_header(curr);
}


/*
 * mem_extend_tail tries to utilize HEAP_TAIL
 * only called when tail_bsize <= user_bsize
 *
*/
void* mem_extend_tail(header* tail_header, size_t tail_bsize, size_t user_bsize) {
	size_t bsize_to_request = user_bsize - tail_bsize;
	void* curr = sbrk(bsize_to_request);
	if (curr == (void*)-1) {
		return NULL;
	}
	HEAP_TAIL = (void*) ((char*) curr + bsize_to_request);
	mem_unchain(tail_header);
	mem_confine(tail_header, user_bsize);
	mem_set(tail_header);
	return mem_get_user_from_header(tail_header);
}

void* mem_dispense(size_t size) {
	header* curr;	
	size_t user_bsize = mem_plan(size);
	for (curr = FREE_HEAD; curr != NULL; curr = curr->next) {
		size_t curr_bsize = mem_header_realsize(curr);
		if (curr_bsize < user_bsize) {
			continue;
		} else if (curr_bsize == user_bsize) {
			mem_unchain(curr);
			mem_confine(curr, curr_bsize);
			mem_set(curr);
			return mem_get_user_from_header(curr);
		} else { //curr_bsize > user_bsize
			size_t remainder = curr_bsize - user_bsize;
			if (remainder <= FRAG_LIMIT) {
				mem_unchain(curr);
				mem_confine(curr, curr_bsize);
				mem_set(curr);
				return mem_get_user_from_header(curr);
			} else {
				return mem_frag_malloc(curr, user_bsize);
			}
		}
	}
	if (HEAP_TAIL != NULL) {
		footer* tail_footer = mem_get_footer_from_end(HEAP_TAIL);
		header* tail_header = mem_get_header_from_footer(tail_footer);
		size_t tail_bsize = mem_footer_realsize(tail_footer);
		if (tail_bsize > user_bsize && !mem_header_check(tail_header)) {
			fprintf(stderr, "wtf is going on");
		}
		if (tail_bsize < user_bsize && !mem_header_check(tail_header)) {
			return mem_extend_tail(tail_header, tail_bsize, user_bsize);
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
	if (malloc_result == NULL) {
		return NULL;
	}
	memset(malloc_result, 0, num * size);
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
	header* curr = mem_get_header_from_user(ptr);
	mem_unset(curr);
	mem_combine(curr);
	return;	
}

void* mem_combine_next_realloc(header* curr_header, header* next_header, size_t comb_bsize, size_t dsize) {
	mem_unchain(next_header);
	mem_confine(curr_header, comb_bsize);
	mem_set(curr_header);
	size_t remainder = comb_bsize - dsize;
	if (remainder <= FRAG_LIMIT) {
		return mem_get_user_from_header(curr_header);
	} else {
		return mem_frag_realloc(curr_header, dsize);
	}
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
: FAILED=(92)
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
	size_t desired_bsize = mem_plan(size);
	header* curr_header = mem_get_header_from_user(ptr);
	size_t curr_bsize = mem_header_realsize(curr_header);
	size_t curr_asize = mem_header_usersize(curr_header);
	if (curr_bsize > desired_bsize) {
		size_t remainder = curr_bsize - desired_bsize;
		if (remainder <= FRAG_LIMIT) {
			return ptr;
		} else {
			return mem_frag_realloc(curr_header, desired_bsize);
		}	
	} else if (curr_bsize == desired_bsize) {
		return ptr;
	} else {
		if (!mem_check_TAIL(curr_header)) {
			header* next_header = mem_get_end_from_header(curr_header);
			size_t comb_bsize = curr_bsize + mem_header_realsize(next_header);
			if (comb_bsize >= desired_bsize && !mem_header_check(next_header)) {
				return mem_combine_next_realloc(curr_header, next_header, comb_bsize, desired_bsize);
			} 
		}
		
		void* malloc_result = malloc(size);
		if (malloc_result == NULL) {
			return NULL;
		}
		memcpy(malloc_result, ptr, curr_asize);
		free(ptr);
		return malloc_result;
	} 
}
