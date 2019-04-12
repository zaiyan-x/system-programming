/**
 * Mad Mad Access Patterns
 * CS 241 - Spring 2019
 * Collab with Eric Wang - wcwang2
 */

#include "tree.h"
#include "utils.h"
#include <stdlib.h> 
#include <string.h> 

/*
  Look up a few nodes in the tree and print the info they contain.
  This version uses fseek() and fread() to access the data.
  ./lookup1 <data_file> <word> [<word> ...]
*/

static char* data_file; 
static FILE* f; 

void search(uint32_t offset, char* word) { 
	if (offset == 0) { 
		printNotFound(word); 
		return; 
	} 
	if (fseek(f, (long int)offset, SEEK_SET)) { 
		formatFail(data_file); 
		return; 
	} 
	BinaryTreeNode* node = malloc(sizeof(BinaryTreeNode));  
	fread(node, sizeof(BinaryTreeNode), 1, f); 
	if (fseek(f, offset + sizeof(BinaryTreeNode), SEEK_SET)) { 
		formatFail(data_file); 
		return; 
	}
	char word2[1024]; 
	fread(word2, 1, 1024, f); 
	int result = strcmp(word, word2); 
	if (result < 0) search(node->left_child, word); 
	else if (result > 0) search(node->right_child, word); 
	else printFound(word, node->count, node->price); 
	free(node); 
} 

int main(int argc, char **argv) {
	if (argc < 3) { 
		printArgumentUsage(); 
		exit(1); 
	} 
	data_file = argv[1]; 
	f = fopen(data_file, "r"); 
	if (!f) { 
		openFail(data_file); 
		exit(2); 
	} 
	char prefix[4]; 
	fread(prefix, 1, 4, f); 
	if (strncmp(prefix, "BTRE", 4)) { 
		formatFail(data_file); 
		exit(2); 
	} 
	for (int i = 2; i < argc; i++) search(4, argv[i]);  
	fclose(f); 
    return 0;
}
