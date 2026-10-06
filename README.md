# memory_allocation
This is a project for me to learn how memory allocation works in C. All credit for the project to https://arjunsreedharan.org/post/148675821737/memory-allocators-101-write-a-simple-memory.

This README will serve as an explanation of memory allocation in C in my own words.

Before beginning with the code, let's go over how memory is laid out in the first place. A process runs within its own virtual address space that is distinct from the virtual address spaces of other processes. This virtual address space comprises of 5 sections:
- Text section - The part that has the binary instructions to be executed by the processor.
- Data section - Contains non-zero initialized static data.
- BSS (Block Started by Symbol) - Contains zero-initialized static data. Static data uninitialized in program is initialized 0 and goes here.
- Heap - Contains the dynamically allocated data.
- Stack - Contains the automatic variables, function arguments, copy of base pointer, etc.

Visualization from the article:

<img width="360" height="303" alt="image" src="https://github.com/user-attachments/assets/c2c64148-7c67-4d31-a06b-ae920122f480" />

As shown in the image, the stack and heap grow in opposing directions. At the end of the heap is a pointer named program break or brk. If we want to allocate more memory to the heap, we need to request the system to increment brk and to release memory, we need to request the system to decrement brk.

In C, we can make use of the system call `sbrk()` to manipulate the program break.
- Calling `sbrk(0)` gives the current address of the program break.
- Calling `sbrk(x)` increments brk by x bytes, as a result allocating memory.
- Calling `sbrk(-x)` decrements brk by x bytes, as a result releasing memory.
- On failure, `sbrk()` returns `(void*) -1`.

The memory in the heap space is structured through a linked list of memory blocks, with each block holding 3 pieces of information: its size, whether it is free, and a pointer to the next block, and each block having a header of a unified size of 16 bytes for keeping track of which block is which. We keep a head and tail pointer to keep track of this list when adding or removing memory from the heap.

To prevent two or more threads from concurrently accessing memory, we put a basic locking mechanism in place. It is a global lock and before every action on the memory you have to get the lock, and once you are done you have to release the lock.

Now we can begin going over the memory allocation functions themselves.

#### malloc()
The `malloc(size)` function allocates _size_ bytes of memory and returns a pointer to the allocated memory. It does this by first checking if the requested size is zero. If it is, then it returns `NULL`. For a valid size, we first get the lock, then we call the function `get_free_block()` which traverses the linked list of memory blocks and looks for an existent memory block that is both free and can hold the specified size + the header size. If a block is found, we mark that block as not free, release the lock, and then return a pointer to that block. If we do not find a block, then we have to extend the heap with `sbrk()`. The heap is extended by a size that fits the requested size as well as the header size.

#### free()
The `free()` function determines if the block we want to free is at the end of the heap. If it is, we can release it to the OS. Otherwise, all we do is mark it "free" in case we want to use that space later. It works by first getting the header of the block we want to free. Then it uses `sbrk(0)` to check if the block to be freed is at the end of the heap. If it is at the end, then we shrink the size of heap and release the memory to the OS by resetting the head and tail pointers to reflect the loss of the last block, calculating the amount of memory to be released, and calling `sbrk()` with the negative of the calculated value. If it is not at the end, we set the block's `is_free` value.

#### calloc()
The `calloc(num, nsize)` function allocates memory for an array of _num_ elements of _nsize_ bytes each and returns a pointer to the allocated memory. It does this by checking for multiplicative overflow, calling `malloc()`, and then clearing the allocated memory to all zeros using `memset()`.

#### realloc()
The `realloc()` function changes the size of a given memory block to the size given. It works by getting the block's header and seeing if the block already has the size to accommodate the requested size. If it does, there's nothing to be done. If it does not, then we call `malloc()` to get a block of the requested size, and relocate contents to the new block using `memcpy()`. The old memory block is freed.
