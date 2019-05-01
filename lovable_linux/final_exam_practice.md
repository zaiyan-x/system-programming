# Angrave's 2019 Acme-CS241-Exam Prep		
## AKA Preparing for the Final Exam & Beyond CS 241... 

Some of the questions require research (wikibook; websearch; wikipedia). 
It is ok to work together & discuss answers! 
Be ready to discuss and clear confusions & misconceptions in your last discussion section.
The final will also include pthreads, fork-exec-wait questions and virtual memory address translation. 
Be awesome. Angrave.

Collaborated with Eric Wang - wcwang2 

## 1. C 


1.	What are the differences between a library call and a system call? Include an example of each.

	a. System call are handled directly by the kernel and therefore is more expensive. An example would be “ssize_t write(int fd, const void* buf, size_t count)”.  
	
	b. Library call are handled by dynamic libraries, and are sometimes used as a wrapper function. An example would be “fprintf(FILE* stream, const char* format, …)”. 

2.	What is the `*` operator in C? What is the `&` operator? Give an example of each.
	‘*’ dereferences a pointer, while ‘&’ retrieves the address of a variable. 
	For example:  
	```
    int a = 5;  
    int* int_ptr = &a; \\ int_ptr is the address of the variable a  
    *int_ptr = 1; \\ ‘*’ dereferences the pointer and changes its value  
	``` 

3.	When is `strlen(s)` != `1+strlen(s+1)` ? 

	If the string is not NULL terminated, the function ‘size_t strlen(const char* s)’ may result in undefined behaviour. 

4.	How are C strings represented in memory? What is the wrong with `malloc(strlen(s))` when copying strings?

	They are represented as a char pointer (in fact a contiguous chunk of memory filled with characters), and ends with a NULL byte. Copying strings with malloc(strlen(s)) ignores its NULL byte, and the copied string may not be NULL terminated. 

5.	Implement a truncation function `void trunc(char*s,size_t max)` to ensure strings are not too long with the following edge cases.  
	```
	if (length < max)
    	strcmp(trunc(s, max), s) == 0
	else if (s is NULL)
    	trunc(s, max) == NULL
	else
    	strlen(trunc(s, max)) <= max
	    // i.e. char s[]="abcdefgh; trunc(s,3); s == "abc". 
	```
	```
	void trunc(char* s, size_t max) { 
    	if (!s) return; // ensures that s is not NULL  
		if (strlen(s) > max) s[max] = 0; 
	} 
	```

6.	Complete the following function to create a deep-copy on the heap of the argv array. Set the result pointer to point to your array. The only library calls you may use are malloc and memcpy. You may not use strdup.

    `void duplicate(char **argv, char ***result);` 

	``` 
	void duplicate(char** argv, char*** result) { 
	    size_t count = 0; 
	    char** tempt = argv; 
	    while (tempt) { 
	        count++; 
	        tempt++; 
	    } 
	    *result = calloc(sizeof(char*),  count + 1); 
	    for (size_t i = 0; i < count; i++) { 
	        size_t length = strlen(argv[i]); 
	        (*result)[i] = calloc(1, length + 1); 
	        memcpy((*result)[i], argv[i], length); 
	    } 
	    (*result)[count] = NULL; 
	} 
	``` 

7.	Write a program that reads a series of lines from `stdin` and prints them to `stdout` using `fgets` or `getline`. Your program should stop if a read error or end of file occurs. The last text line may not have a newline char.
	
	```
	ssize_t read = 0; 
	size_t len = 0; 
	char* command = NULL; 
	bool newline = false; 
	while ((read = getline(&command, &len, stdin)) != -1) { 
	    if (newline) fprintf(stdout, '\n'); 
	    else newline = true; 
	    fprintf(stdout, command); 
	} 
	```
## 2. Memory 

1.	Explain how a virtual address is converted into a physical address using a multi-level page table. You may use a concrete example e.g. a 64bit machine with 4KB pages.

	A virtual address stores the address of a variable (potentially another address). To retrieve the real physical address, you may have to go through multiple levels of indirection. 
2.	Explain Knuth's and the Buddy allocation scheme. Discuss internal & external Fragmentation.

	Buddy allocation scheme divides the heap into segregated memory group with different sizes. So that the request for heap memory will be handled according to the size being asked.
	Internal fragmentation may come from the implementation of the Buddy allocation scheme, as it will only assign memory chuck with size of power of 2. A request of 68B will be assigned with 128B heap, causing unused space, or fragmentation, within memory. 

	External fragmentation is attributed to certain memory allocation scheme. It happens when freed memory is interspersed by allocated memory, so even if there is free space, it is not usable because the memory is too divided.
3.	What is the difference between the MMU and TLB? What is the purpose of each? 

	MMU is a hardware unit which translates virtual address to physical memory address, while TLB is used to avoid the necessity of accessing the main memory everytime a virtual address is mapped. 
4.	Assuming 4KB page tables what is the page number and offset for virtual address 0x12345678? 

	Page number: 0x12345, offset: 0x678 
5.	What is a page fault? When is it an error? When is it not an error? 

	Page fault happens when a running program tries to access virtual memory in its address space that is not mapped to physical memory. Page fault becomes an error when the user tries to write to a non-writeable piece of memory, while it is not an error when there is not mapping yet but is a valid address or is yet in memory. 
6.	What is Spatial and Temporal Locality? Swapping? Swap file? Demand Paging? 

	Temporal locality refers to the reuse of specific data within a small duration. Spatial locality refers to the use of data elements within close storage locations. 

	Swapping is a memory reclamation method, in which memory that are not currently in use are swapped to a disk to make the memory available for other applications or processes. A swap file allows an operating system to use hard disk space to simulate extra memory. Demand paging is a type of swapping in which pages of data are not copied from disk to RAM until they are needed.

## 3. Processes and Threads 

1.	What resources are shared between threads in the same process?

	Heap, address space, variables and file descriptors. 

2.	Explain the operating system actions required to perform a process context switch

	The OS saves the context of the current process on the CPU, and selects a new process for execution. It will then update the selected process' control block, including changing its state to run, and updates the memory management data structure. 

3.	Explain the actions required to perform a thread context switch to a thread in the same process

	The kernel will assign the thread to a CPU for a short duration or until it runs out of things to do, and automatically switches the CPU to work on another thread. 

4.	How can a process be orphaned? What does the process do about it?

	An orphaned process is created when its parent process has finished and / or terminated, while itself remains running. Orphaned processes are adopted by the “init” process, which will wait on its children. 

5.	How do you create a process zombie?

	A zombie process is created when a child process has completed execution, but still has an entry in the process table. This would exhaust the process table,  which is a finite resource, and excessive zombie processes can fill it up. 

6.	Under what conditions will a multi-threaded process exit? (List at least 4) 

	a. Returning from the thread function 

	b. Calling "pthread_exit" 

	c. Cancelling the thread with "pthread_cancel" 

	d. Terminating the process through a signal 

	e. Calling exit() or abort() 

	f. Returning from main() 

	g. "exec"ing another program 

	h. Undefined behaviours may terminate threads 

## 4. Scheduling 
1.	Define arrival time, pre-emption, turnaround time, waiting time and response time in the context of scheduling algorithms. What is starvation?  Which scheduling policies have the possibility of resulting in starvation?

2.	Which scheduling algorithm results the smallest average wait time?

3.	What scheduling algorithm has the longest average response time? shortest total wait time?

4.	Describe Round-Robin scheduling and its performance advantages and disadvantages.

5.	Describe the First Come First Serve (FCFS) scheduling algorithm. Explain how it leads to the convoy effect. 

6.	Describe the Pre-emptive and Non-preemptive SJF scheduling algorithms. 

7.	How does the length of the time quantum affect Round-Robin scheduling? What is the problem if the quantum is too small? In the limit of large time slices Round Robin is identical to _____?

8.	What reasons might cause a scheduler switch a process from the running to the ready state?

## 5. Synchronization and Deadlock

1.	Define circular wait, mutual exclusion, hold and wait, and no-preemption. How are these related to deadlock?

2.	What problem does the Banker's Algorithm solve?

3.	What is the difference between Deadlock Prevention, Deadlock Detection and Deadlock Avoidance?

4.	Sketch how to use condition-variable based barrier to ensure your main game loop does not start until the audio and graphic threads have initialized the hardware and are ready.

5.	Implement a producer-consumer fixed sized array using condition variables and mutex lock.

6.	Create an incorrect solution to the CSP for 2 processes that breaks: i) Mutual exclusion. ii) Bounded wait.

7.	Create a reader-writer implementation that suffers from a subtle problem. Explain your subtle bug.

## 6. IPC and signals

1.	Write brief code to redirect future standard output to a file.

2.	Write a brief code example that uses dup2 and fork to redirect a child process output to a pipe

3.	Give an example of kernel generated signal. List 2 calls that can a process can use to generate a SIGUSR1.

4.	What signals can be caught or ignored?

5.	What signals cannot be caught? What is signal disposition?

6.	Write code that uses sigaction and a signal set to create a SIGALRM handler.

7.	Why is it unsafe to call printf, and malloc inside a signal handler?

## 7. Networking 

1.	Explain the purpose of `socket`, `bind`, `listen`, and `accept` functions

2.	Write brief (single-threaded) code using `getaddrinfo` to create a UDP IPv4 server. Your server should print the contents of the packet or stream to standard out until an exclamation point "!" is read.

3.	Write brief (single-threaded) code using `getaddrinfo` to create a TCP IPv4 server. Your server should print the contents of the packet or stream to standard out until an exclamation point "!" is read.

4.	Explain the main differences between using `select` and `epoll`. What are edge- and level-triggered epoll modes?

5.	Describe the services provided by TCP but not UDP. 

6.	How does TCP connection establishment work? And how does it affect latency in HTTP1.0 vs HTTP1.1?

7.	Wrap a version of read in a loop to read up to 16KB into a buffer from a pipe or socket. Handle restarts (`EINTR`), and socket-closed events (return 0).

8.	How is Domain Name System (DNS) related to IP and UDP? When does host resolution not cause traffic?

9.	What is NAT and where and why is it used? 

## 8. Files 

1.	Write code that uses `fseek`, `ftell`, `read` and `write` to copy the second half of the contents of a file to a `pipe`.

2.	Write code that uses `open`, `fstat`, `mmap` to print in reverse the contents of a file to `stderr`.

3.	Write brief code to create a symbolic link and hard link to the file /etc/password

4.	"Creating a symlink in my home directory to the file /secret.txt succeeds but creating a hard link fails" Why? 

5.	Briefly explain permission bits (including sticky and setuid bits) for files and directories.

6.	Write brief code to create a function that returns true (1) only if a path is a directory.

7.	Write brief code to recursive search user's home directory and sub-directories (use `getenv`) for a file named "xkcd-functional.png' If the file is found, print the full path to stdout.

8.	The file 'installmeplz' can't be run (it's owned by root and is not executable). Explain how to use sudo, chown and chmod shell commands, to change the ownership to you and ensure that it is executable.

## 9. File system 
Assume 10 direct blocks, a pointer to an indirect block, double-indirect, and triple indirect block, and block size 4KB.

1.	A file uses 10 direct blocks, a completely full indirect block and one double-indirect block. The latter has just one entry to a half-full indirect block. How many disk blocks does the file use, including its content, and all indirect, double-indirect blocks, but not the inode itself? A sketch would be useful.

2.	How many i-node reads are required to fetch the file access time at /var/log/dmesg ? Assume the inode of (/) is cached in memory. Would your answer change if the file was created as a symbolic link? Hard link?

3.	What information is stored in an i-node?  What file system information is not? 

4.	Using a version of stat, write code to determine a file's size and return -1 if the file does not exist, return -2 if the file is a directory or -3 if it is a symbolic link.

5.	If an i-node based file uses 10 direct and n single-indirect blocks (1 <= n <= 1024), what is the smallest and largest that the file contents can be in bytes? You can leave your answer as an expression.

6.	When would `fstat(open(path,O_RDONLY),&s)` return different information in s than `lstat(path,&s)`?

## 10. "I know the answer to one exam question because I helped write it"

Create a hard but fair 'spot the lie/mistake' multiple choice or short-answer question. Ideally, 50% can get it correct.
