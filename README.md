# memory_allocation
This is a project for me to learn how memory allocation works in C. All credit for the project to https://arjunsreedharan.org/post/148675821737/memory-allocators-101-write-a-simple-memory.

This README will serve as an explanation of memory allocation in C in my own words.

Before beginning with the code, let's go over how memory is laid out in the first place. A process runs within its own virtual address space that is distinct from the virtual address spaces of other processes. This virtual address space comprises of 5 sections:
- Text section - The part that has the binary instructions to be executed by the processor.
- Data section - Contains non-zero initialized static data.
- BSS (Block Started by Symbol) - Contains zero-initialized static data. Static data uninitialized in program is initialized 0 and goes here.
- Heap - Contains the dynamically allocated data.
- Stack - Contains the automatic variables, function arguments, copy of base pointer, etc.

Visualization:

<img width="360" height="303" alt="image" src="https://github.com/user-attachments/assets/c2c64148-7c67-4d31-a06b-ae920122f480" />

As shown in the image, the stack and heap grow in opposing directions. At the end of the heap is a pointer named program break or brk. If we want to allocate more memory to the heap, we need to request the system to increment brk and to release memory, we need to request the system to decrement brk.

In C, we can make use of the system call `sbrk()` to manipulate the program break.
- Calling `sbrk(0)` gives the current address of the program break.
- Calling `sbrk(x)` increments brk by x bytes, as a result allocating memory.
- Calling `sbrk(-x)` decrements brk by x bytes, as a result releasing memory.
- On failure, `sbrk()` returns `(void*) -1`.

Now we can begin going over the memory allocation functions themselves.

###### malloc()
The `malloc()` function allocates bytes of memory and returns a pointer to the allocated memory.

###### free()
The `free()` function determines if the block we want to free is at the end of the heap. If it is, we can release it
