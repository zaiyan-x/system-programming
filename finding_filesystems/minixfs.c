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
	
	minixfs_dirent child_dirent = {child_filename, child_inode_number};
	char * dirent_string = malloc( MAX_DIR_NAME_LEN );
	make_string_from_dirent(dirent_string, child_dirent);
	off_t * off = malloc(sizeof(off_t));
	*off = (off_t) child_inode->size;
	minixfs_write(fs, path, dirent_string, MAX_DIR_NAME_LEN, off);
	free(dirent_string);
	free(off);
	return child_inode;
}

ssize_t minixfs_virtual_read(file_system *fs, const char *path, void *buf,
                             size_t count, off_t *off) {
    if (!strcmp(path, "info")) {
        // TODO implement the "info" virtual file here
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
	if ((count + (size_t)(*off)) % sizeof(data_block) != 0) block_count++; 
	if (minixfs_min_blockcount(fs, path, block_count) == -1) {
		errno = ENOSPC;
		return -1;
	}

	size_t written_block_count = (size_t)(*off) / sizeof(data_block);
	size_t offset = (size_t)(*off) % sizeof(data_block);
	
	inode * file_inode = get_inode(fs, path);	
	if (file_inode -> size < *off + count) file_inode -> size = *off + count; 
	clock_gettime(CLOCK_REALTIME, &file_inode -> atim); 
	clock_gettime(CLOCK_REALTIME, &file_inode -> mtim); 
	int indirect = 0; 

	data_block * current_data_block;
	if (written_block_count < NUM_DIRECT_INODES) {
		current_data_block = fs->data_root + file_inode->direct[written_block_count];
	} else { // indirect
		written_block_count -= NUM_DIRECT_INODES;
		current_data_block = fs->data_root + file_inode->indirect + written_block_count;
		indirect = 1; 
	}
	
	size_t count2 = count; 
	size_t need; 
	size_t copied = 0; 
	if (offset != 0) {
		need = ((sizeof(data_block) - offset) < count2) ? sizeof(data_block) - offset : count2; 
		memcpy((char*)current_data_block + offset, buf, need); 
		count2 -= need; 
		copied += need; 
		if (written_block_count == NUM_DIRECT_INODES - 1) indirect = 1; 
		written_block_count++; 
		buf += need; 
	} 
	if (count2 == 0) { 
		*off += copied; 
		return (ssize_t)copied; 
	} 
	while (indirect == 0 && count2 > 0) { 
		current_data_block = fs -> data_root + file_inode -> direct[written_block_count]; 
		need = (sizeof(data_block) < count2) ? sizeof(data_block) : count2; 
		copied += need; 
		count2 -= need; 
		memcpy(current_data_block, buf, need); 
		buf += need; 
		written_block_count++; 
		if (written_block_count == NUM_DIRECT_INODES) { 
			written_block_count -= NUM_DIRECT_INODES; 
			indirect = 1; 
		} 
	} 
	if (count2 == 0) { 
		*off += copied; 
		return (ssize_t)copied; 
	} 
	while (1) { 
		current_data_block = fs -> data_root + file_inode -> indirect + written_block_count; 
		need = (sizeof(data_block) < count2) ? sizeof(data_block) : count2; 
		copied += need; 
		count2 -= need; 
		memcpy(current_data_block, buf, need); 
		buf += need; 
		written_block_count++; 
		if (count2 == 0) break; 
	} 
	*off += copied; 
	return copied; 
}

ssize_t minixfs_read(file_system *fs, const char *path, void *buf, size_t count,
                     off_t *off) {
    const char *virtual_path = is_virtual_path(path);
    if (virtual_path)
        return minixfs_virtual_read(fs, virtual_path, buf, count, off);
    // 'ere be treasure!
    return -1;
}
