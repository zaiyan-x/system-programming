/**
 * Mad Mad Access Patterns
 * CS 241 - Spring 2019
 * Collab with Eric Wang - wcwang2
 */

#include "tree.h"
#include "utils.h"
#include <sys/mman.h> 
#include <sys/stat.h> 
#include <string.h> 
#include <stdlib.h> 

/*
  Look up a few nodes in the tree and print the info they contain.
  This version uses mmap to access the data.
  ./lookup2 <data_file> <word> [<word> ...]
*/

static char* data_file; 
static void* data; 

void search(uint32_t offset, char* word) { 
	if (offset == 0) { 
		printNotFound(word); 
		return; 
	} 
	BinaryTreeNode* node = (BinaryTreeNode*)(data + offset); 
	int result = strcmp(word, node -> word); 
	if (result < 0) search(node -> left_child, word); 
	else if (result > 0) search(node -> right_child, word); 
	else printFound(word, node -> count, node -> price); 
} 

int main(int argc, char **argv) {
    if (argc < 3) { 
		printArgumentUsage(); 
		exit(1); 
	} 
	data_file = argv[1]; 
	FILE* f = fopen(data_file, "r"); 
	if (!f) { 
		openFail(data_file); 
		exit(2); 
	} 
	struct stat buf; 
	fstat(fileno(f), &buf); 
	off_t size = buf.st_size; 
	data = mmap(0, size, PROT_READ, MAP_SHARED, fileno(f), 0); 
	if (data == MAP_FAILED) { 
		mmapFail(data_file); 
		exit(1); 
	} 
	fclose(f); 
	if (strncmp(data, "BTRE", 4)) { 
		formatFail(data_file); 
		exit(2); 
	} 
	for (int i = 2; i < argc; i++) search(4, argv[i]); 
	return 0;
}
