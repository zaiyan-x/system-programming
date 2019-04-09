/**
 * Finding Filesystems
 * CS 241 - Spring 2019
 */
#include "minixfs.h"
#include "minixfs_utils.h"
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h> 

#define MIN(x, y) (x < y ? x : y)

/**
 * Virtual paths:
 *  Add your new virtual endpoint to minixfs_virtual_path_names
 */
char *minixfs_virtual_path_names[] = {"info", /* add your paths here*/};

/**
 * Forward declaring block_info_string so that we can attach unused on it
 * This prevents a compiler warning if you haven't used it yet.
 *
 * This function generates the info string that the virtual endpoint info should
 * emit when read
 */
static char *block_info_string(ssize_t num_used_blocks) __attribute__((unused));
static char *block_info_string(ssize_t num_used_blocks) {
    char *block_string = NULL;
    ssize_t curr_free_blocks = DATA_NUMBER - num_used_blocks;
    asprintf(&block_string, "Free blocks: %zd\n"
                            "Used blocks: %zd\n",
             curr_free_blocks, num_used_blocks);
    return block_string;
}

// Don't modify this line unless you know what you're doing
int minixfs_virtual_path_count =
    sizeof(minixfs_virtual_path_names) / sizeof(minixfs_virtual_path_names[0]);

int minixfs_chmod(file_system *fs, char *path, int new_permissions) {
	inode * file_inode = get_inode(fs, path);
	int retval = 0;
	if (file_inode == NULL) {
		retval = -1;
		errno = ENOENT;
	} else {
		int type = file_inode->mode >> RWX_BITS_NUMBER;
		file_inode->mode = new_permissions | type << RWX_BITS_NUMBER;
		clock_gettime(CLOCK_REALTIME, &file_inode->ctim);
		retval = 0;
	}
	return retval;
}

int minixfs_chown(file_system *fs, char *path, uid_t owner, gid_t group) {
	inode * file_inode = get_inode(fs, path);
	int retval = 0;
	if (file_inode == NULL) {
		retval = -1;
		errno = ENOENT;
	} else {
		if (owner != ((uid_t)-1)) {
			file_inode->uid = owner;
		}
		if (group != ((gid_t)-1)) {
			file_inode->gid = group;
		}
		clock_gettime(CLOCK_REALTIME, &file_inode->ctim);
		retval = 0;
	} 
	return retval;
}

inode *minixfs_create_inode_for_path(file_system *fs, const char *path) {
	inode * child_inode = get_inode(fs, path);
	if (child_inode != NULL) {
		return NULL;
	}
	const char * filename;
	inode * parent_inode = parent_directory(fs, path, &filename);
	if (valid_filename(filename) != 1) { //filename derived from path is invalid; return NULL
		return NULL;
	}
	
	inode_number child_inode_number = first_unused_inode(fs);
	if (child_inode_number == -1) { //file cannot be created
		return NULL;
	}

	//all clear: proceed to create
	child_inode = fs->inode_root + child_inode_number;
	init_inode(parent_inode, child_inode);
	char child_filename[FILE_NAME_LENGTH];
	strcpy(child_filename, filename);
	char parent_path[MAX_DIR_NAME_LEN];
	strcpy(parent_path, path);
	char * last_slash_occurance = strrchr(parent_path, '/');
	last_slash_occurance = 0;
		
	minixfs_dirent child_dirent = {child_filename, child_inode_number};
	char * dirent_string = malloc( MAX_DIR_NAME_LEN );
	make_string_from_dirent(dirent_string, child_dirent);
	off_t * off = malloc(sizeof(off_t));
	*off = (off_t) child_inode->size;
	minixfs_write(fs, parent_path, dirent_string, MAX_DIR_NAME_LEN, off);
	free(dirent_string);
	free(off);
	return child_inode;
}

ssize_t minixfs_virtual_read(file_system *fs, const char *path, void *buf,
                             size_t count, off_t *off) {
    if (!strcmp(path, "info")) {
        // TODO implement the "info" virtual file here
		uint64_t i;
		ssize_t used_block = 0;
		for (i = 0; i < fs->meta->dblock_count; i++) {
			if (get_data_used(fs, i) == 1) { //in use
				used_block++;
			}	
		}
		char * virtual_info = block_info_string(used_block);
		if ((size_t)*off >= strlen(virtual_info)) {
			return 0;
		}
		size_t total_bytes_to_read = MIN(count, strlen(virtual_info));
		*off += total_bytes_to_read;
		memcpy(buf, virtual_info, total_bytes_to_read);
		free(virtual_info);
		return total_bytes_to_read;	
    }
    // TODO implement your own virtual file here
    errno = ENOENT;
    return -1;
}

ssize_t minixfs_write(file_system *fs, const char *path, const void *buf,
                      size_t count, off_t *off) {
	
    size_t max_possible_file_size = (NUM_DIRECT_INODES + NUM_INDIRECT_INODES) * sizeof(data_block);
	if (count + *off > max_possible_file_size) { //SOB is asking for too much
		errno = ENOSPC;
		return -1;
	}
	size_t block_count = (count + (size_t) (*off)) / sizeof(data_block);
	if ((count + (size_t)(*off)) % sizeof(data_block) != 0) {
		block_count++; 
	}
	if (minixfs_min_blockcount(fs, path, block_count) == -1) {
		errno = ENOSPC;
		return -1;
	}

	// prepare to memcpy
	size_t written_block_count = (size_t)(*off) / sizeof(data_block);
	size_t total_bytes_to_write = count;
	size_t total_bytes_written = 0;
	size_t curr_bytes_to_write = 0;
	size_t curr_bytes_able_to_write = 0;
	inode * file_inode = get_inode(fs, path);
	if (file_inode == NULL) {
		minixfs_create_inode_for_path(fs, path);
	}
	void * curr_data_block;
	
	if (file_inode->size < *off + total_bytes_to_write) {
		file_inode->size = *off + total_bytes_to_write;
	}
	clock_gettime(CLOCK_REALTIME, &file_inode->atim);
	clock_gettime(CLOCK_REALTIME, &file_inode->mtim);
	
	while (written_block_count < NUM_DIRECT_INODES && total_bytes_to_write > 0) {
		curr_data_block = fs->data_root + file_inode->direct[written_block_count];
		curr_data_block = (char*) curr_data_block + ((size_t)(*off) % sizeof(data_block));	
		curr_bytes_able_to_write = sizeof(data_block) - ((size_t)(*off) % sizeof(data_block));
		curr_bytes_to_write = (curr_bytes_able_to_write < total_bytes_to_write) ? curr_bytes_able_to_write : total_bytes_to_write;
		memcpy(curr_data_block, buf, curr_bytes_to_write);
		*off += curr_bytes_to_write;
		total_bytes_to_write -= curr_bytes_to_write;
		total_bytes_written += curr_bytes_to_write;
		buf += curr_bytes_to_write;
		written_block_count++;	
	}
	// Adjust block for indirect blocks
	written_block_count -= NUM_DIRECT_INODES;
	data_block_number curr_data_block_offset = 0;
	while (total_bytes_to_write > 0) {
		curr_data_block_offset = * (data_block_number*) ((char*) (fs->data_root + file_inode->indirect) + sizeof(data_block_number) * written_block_count);
		curr_data_block = fs->data_root + curr_data_block_offset;
		curr_data_block = (char*) curr_data_block + ((size_t)(*off) % sizeof(data_block));
		curr_bytes_able_to_write = sizeof(data_block) - ((size_t)(*off) % sizeof(data_block));
		curr_bytes_to_write = (curr_bytes_able_to_write < total_bytes_to_write) ? curr_bytes_able_to_write : total_bytes_to_write;
		memcpy(curr_data_block, buf, curr_bytes_to_write);
		*off += curr_bytes_to_write;
		total_bytes_to_write -= curr_bytes_to_write;
		total_bytes_written += curr_bytes_to_write;
		buf += curr_bytes_to_write;
		written_block_count++;
	}
	
	return total_bytes_written; 
}

ssize_t minixfs_read(file_system *fs, const char *path, void *buf, size_t count,
                     off_t *off) {
    const char *virtual_path = is_virtual_path(path);
    if (virtual_path)
        return minixfs_virtual_read(fs, virtual_path, buf, count, off);
  	inode * file_inode = get_inode(fs, path);
	if (file_inode == NULL) { //file does not exist
		errno = ENOENT;
		return -1;
	}
	if (file_inode->size < (size_t)*off) {
		return 0;
	}

	size_t total_bytes_to_read = MIN(count, file_inode->size - *off);
	size_t total_bytes_read = 0;
	size_t curr_bytes_to_read = 0;
	size_t curr_bytes_able_to_read = 0;
	size_t read_block_count = *off / sizeof(data_block);
	void * curr_data_block = NULL;

	while (read_block_count < NUM_DIRECT_INODES && total_bytes_to_read) {
		curr_data_block = fs->data_root + file_inode->direct[read_block_count];
		curr_data_block = (char*) curr_data_block + ((size_t)(*off) % sizeof(data_block));
		curr_bytes_able_to_read = sizeof(data_block) - ((size_t)(*off) % sizeof(data_block));
		curr_bytes_to_read = (curr_bytes_able_to_read < total_bytes_to_read) ? curr_bytes_able_to_read : total_bytes_to_read;
		memcpy(buf, curr_data_block, curr_bytes_to_read);
		*off += curr_bytes_to_read;
		total_bytes_to_read -= curr_bytes_to_read;
		total_bytes_read += curr_bytes_to_read;
		buf += curr_bytes_to_read;
		read_block_count++;
	}		
	 
	// Adjust block for indirect blocks
	read_block_count -= NUM_DIRECT_INODES;
	data_block_number curr_data_block_offset = 0;
	while (total_bytes_to_read > 0) {
		curr_data_block_offset = * (data_block_number*) ((char*) (fs->data_root + file_inode->indirect) + sizeof(data_block_number) * read_block_count);
		curr_data_block = fs->data_root + curr_data_block_offset;
		curr_data_block = (char*) curr_data_block + ((size_t)(*off) % sizeof(data_block));
		curr_bytes_able_to_read = sizeof(data_block) - ((size_t)(*off) % sizeof(data_block));
		curr_bytes_to_read = (curr_bytes_able_to_read < total_bytes_to_read) ? curr_bytes_able_to_read : total_bytes_to_read;
		memcpy(buf, curr_data_block, curr_bytes_to_read);
		*off += curr_bytes_to_read;
		total_bytes_to_read -= curr_bytes_to_read;
		total_bytes_read += curr_bytes_to_read;
		buf += curr_bytes_to_read;
		read_block_count++;
	}

	clock_gettime(CLOCK_REALTIME, &file_inode->atim);
	return total_bytes_read;
}
