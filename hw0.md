Homework 0
----------

### Watch the videos and write up your answers to the following questions

**Important!**

The virtual machine-in-your-browser and the videos you need for HW0 are here:

<http://cs-education.github.io/sys/>

Questions? Comments? Use Piazza: <https://piazza.com/illinois/spring2019/cs241>

The in-browser virtual machine runs entirely in Javascript and is fastest in Chrome. Note the VM and any code you write is reset when you reload the page, **so copy your code to a separate document.** The post-video challenges are not part of homework 0 but you learn the most by doing rather than just passively watching - so we suggest you have some fun with each end-of-video challenge.

HW0 questions are below. Please use this document to write the answers. This will be hand graded.

### Chapter 1

In which our intrepid hero battles standard out, standard error, file descriptors and writing to files

1.  **Hello, World! (system call style)** Write a program that uses `write()` to print out “Hi! My name is <Your Name>”.

```c
#include <unistd.h>

int main() {
	write(1, "Hi! My name is Zaiyan Xu.\n", sizeof("Hi! My name is Zaiyan Xu.\n"))
	return 0;
} 
```

2.  **Hello, Standard Error Stream!** Write a function to print out a triangle of height `n` to standard error. Your function should have the signature `void write_triangle(int n)` and should use `write()`. The triangle should look like this, for n = 3:

```
*
**
***
```

```c
void write_triangle(int n) {
	int count;
	for (count = 0; count < n; count++) {
		int count2;
		for (count2 = 0; count2 <= count; count2++) {
			write(2, "*", 1);
		}
		write(2, "\n", 1);
	}
}
```

3.  **Writing to files** Take your program from “Hello, World!” modify it write to a file called `hello_world.txt`. Make sure to to use correct flags and a correct mode for `open()` (`man 2 open` is your friend).

```
#include <unistd.h>
#include <fcntl.h>

int main() {
	mode_t user_mode = S_IRUSR | S_IWUSR;
	int myFile = open("hello_world.txt", O_RDWR | O_TRUNC | O_CREAT, user_mode);
	write(myFile, "Hello World!\n", 13);
	close(myFile);
	return 0;
}
```

4. **Not everything is a system call** Take your program from “Writing to files” and replace `write()` with `printf()`. *Make sure to print to the file instead of standard out!*

```c
#include <unistd.h>
#include <fcntl.h>

int main() {
	mode_t user_mode = S_IRUSR | S_IWUSR;
	close(1);
	int myFile = open("hello_world.txt", O_RDWR | O_TRUNC | O_CREAT, user_mode);
	printf("Hello World! This is writing by printf.\n");
	return 0;
}
```

5.  What are some differences between `write()` and `printf()`?

```c
1. write() is a syscall while printf() is a standard library function, but printf() will eventually call write().
2. printf() has a buffer that holds bytes to be sent while write() (in theory) immediately send given characters to output devices. Thus, write(), as a syscall, is very costly.
3. printf() can do some string processing or let you write data in different formats.
```

### Chapter 2

Sizing up C types and their limits, `int` and `char` arrays, and incrementing pointers

1.  How many bits are there in a byte?

```c
8 bits is one byte.
```

2.  How many bytes are there in a `char`?

```c
A `char` is one byte.
```

3.  How many bytes the following are on your machine? 

* `int`: 4 bytes 
* `double`: 8 bytes 
* `float`: 4 bytes
* `long`: 4 bytes
* `long long`: 8 bytes

4.  On a machine with 8 byte integers, the declaration for the variable `data` is `int data[8]`. If the address of data is `0x7fbd9d40`, then what is the address of `data+2`?

```c

```

5.  What is `data[3]` equivalent to in C? Hint: what does C convert `data[3]` to before dereferencing the address? Remember, the type of a string constant `abc` is an array.

```c
// Your answer here
```

6.  Why does this segfault?

```c
char *ptr = "hello";
*ptr = 'J';
```

7.  What does `sizeof("Hello\0World")` return?

```c
// Your answer here
```

8.  What does `strlen("Hello\0World")` return?

```c
// Your answer here
```

9.  Give an example of X such that `sizeof(X)` is 3.

```c
// Your code here
```

10. Give an example of Y such that `sizeof(Y)` might be 4 or 8 depending on the machine.

```c
// Your code here
```

### Chapter 3

Program arguments, environment variables, and working with character arrays (strings)

1.  What are two ways to find the length of `argv`?

2.  What does `argv[0]` represent?

3.  Where are the pointers to environment variables stored (on the stack, the heap, somewhere else)?

4.  On a machine where pointers are 8 bytes, and with the following code:

    ``` c
    char *ptr = "Hello";
    char array[] = "Hello";
    ```

    What are the values of `sizeof(ptr)` and `sizeof(array)`? Why?

```c
// Your answer here
```

5.  What data structure manages the lifetime of automatic variables?

### Chapter 4

Heap and stack memory, and working with structs

1.  If I want to use data after the lifetime of the function it was created in ends, where should I put it? How do I put it there?

2.  What are the differences between heap and stack memory?

3.  Are there other kinds of memory in a process?

4.  Fill in the blank: “In a good C program, for every malloc, there is a \_\_\_”.

5.  What is one reason `malloc` can fail?

6.  What are some differences between `time()` and `ctime()`?

7.  What is wrong with this code snippet?

``` c
free(ptr);
free(ptr);
```

8.  What is wrong with this code snippet?

``` c
free(ptr);
printf("%s\n", ptr);
```

9.  How can one avoid the previous two mistakes?


10. Use the following space for the next four questions

```c
// 10

// 12

// 13

int main() {
// 11
}
```

* Create a `struct` that represents a `Person`. Then make a `typedef`, so that `struct Person` can be replaced with a single word. A person should contain the following information: their name (a string), their age (an integer), and a list of their friends (stored as a pointer to an array of pointers to `Person`s). 

*  Now, make two persons on the heap, “Agent Smith” and “Sonny Moore”, who are 128 and 256 years old respectively and are friends with each other. Create functions to create and destroy a Person (Person’s and their names should live on the heap).

* `create()` should take a name and age. The name should be copied onto the heap. Use malloc to reserve sufficient memory for everyone having up to ten friends. Be sure initialize all fields (why?).

* `destroy()` should free up not only the memory of the person struct, but also free all of its attributes that are stored on the heap. Destroying one person should not destroy any others.


### Chapter 5

Text input and output and parsing using `getchar`, `gets`, and `getline`.

1.  What functions can be used for getting characters from `stdin` and writing them to `stdout`?

2.  Name one issue with `gets()`.

3.  Write code that parses the string “Hello 5 World” and initializes 3 variables to “Hello”, 5, and “World”.

```c
// Your code here
```

4.  What does one need to define before including `getline()`?

5.  Write a C program to print out the content of a file line-by-line using `getline()`.

```c
// Your code here
```

### C Development

These are general tips for compiling and developing using a compiler and git. Some web searches will be useful here


1.  What compiler flag is used to generate a debug build?


2.  You fix a problem in the Makefile and type `make` again. Explain why this may be insufficient to generate a new build.


3.  Are tabs or spaces used to indent the commands after the rule in a Makefile?


4.  What does `git commit` do? What’s a `sha` in the context of git?


5.  What does `git log` show you?


6.  What does `git status` tell you and how would the contents of `.gitignore` change its output?


7.  What does `git push` do? Why is it not just sufficient to commit with `git commit -m ’fixed all bugs’ `?


8.  What does a non-fast-forward error `git push` reject mean? What is the most common way of dealing with this?


### Optional: Just for fun

-   Convert your a song lyrics into System Programming and C code covered in this wiki book and share on Piazza.

-   Find, in your opinion, the best and worst C code on the web and post the link to Piazza.

-   Write a short C program with a deliberate subtle C bug and post it on Piazza to see if others can spot your bug.

-   Do you have any cool/disastrous system programming bugs you’ve heard about? Feel free to share with your peers and the course staff on piazza.
