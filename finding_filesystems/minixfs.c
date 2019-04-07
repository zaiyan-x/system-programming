/**
 * Finding Filesystems
 * CS 241 - Spring 2019
 */
#include "minixfs.h"
#include "minixfs_utils.h"
#include <errno.h>
#include <stdio.h>
#include <string.h>

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

	//add child dirent to parent directory
	uint64_t parent_size = parent_inode->size;
	size_t num_of_file = parent_size / sizeof(data_block);
	size_t data_block_offset = parent_size % sizeof(data_block);

	if (num_of_file >= NUM_DIRECT_INODES) { //add child_inode to indirect
		
	} else { //add child_inode to direct
		data_block * curr_block = fs->data_root + parent_inode->direct[num_of_file];
		make_string_from_dirent(((char *) curr_block) + data_block_offset, (minixfs_dirent) {child_filename, child_inode_number});
	}	

	parent_inode->size += FILE_NAME_ENTRY; 
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
    // X marks the spot
    return -1;
}

ssize_t minixfs_read(file_system *fs, const char *path, void *buf, size_t count,
                     off_t *off) {
    const char *virtual_path = is_virtual_path(path);
    if (virtual_path)
        return minixfs_virtual_read(fs, virtual_path, buf, count, off);
    // 'ere be treasure!
    return -1;
}
