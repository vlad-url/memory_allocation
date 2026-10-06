// this is just a project for learning //
// all credit to https://arjunsreedharan.org/post/148675821737/memory-allocators-101-write-a-simple-memory //

#include <unistd.h>
#include <string.h>
#include <pthread.h>

typedef char ALIGN[16];

union header {
    struct {
        size_t size;
        unsigned is_free;
        union header* next;
    } s;
    // force the header to be aligned to 16 bytes //
    ALIGN stub;
};
typedef union header header_t;

header_t *head = NULL, *tail = NULL;
pthread_mutex_t global_malloc_lock;

header_t* get_free_block(size_t size) {
    header_t* curr = head;
    while(curr) {
        // look for a free block that can hold the requested size //
        if (curr->s.is_free && curr->s.size >= size)
            return curr;
        curr = curr->s.next;
    }
    return NULL;
}

void free(void* block) {
    header_t *header, *tmp;

    // program breaks at the end of the process's data segment //
    void* programbreak;

    if (!block) return;
    pthread_mutex_lock(&global_malloc_lock);
    header = (header_t*)block - 1;
    
    // sbrk(0) gives the current program break address //
    programbreak = sbrk(0);

    if ((char*)block + header->s.size == programbreak) {
        if (head == tail) {
            head = tail = NULL;
        } else {
            tmp = head;
            while (tmp) {
                if (tmp->s.next == tail) {
                    tmp->s.next = NULL;
                    tail = tmp;
                }
                tmp = tmp->s.next;
            }
        }

        // sbrk() with a negative argument decrements the program break //
        sbrk(0 - header->s.size - sizeof(header_t));
        pthread_mutex_unlock(&global_malloc_lock);
        return;
    }
    header->s.is_free = 1;
    pthread_mutex_unlock(&global_malloc_lock);
}

void* malloc(size_t size) {
    size_t total_size;
    void* block;
    header_t* header;

    if (!size) return NULL;
    pthread_mutex_lock(&global_malloc_lock);

    header = get_free_block(size);
    if (header) { // found a free block
        header->s.is_free = 0;
        pthread_mutex_unlock(&global_malloc_lock);
        return (void*)(header + 1);
    }

    // get the memory to fit in the block //
    total_size = sizeof(header_t) + size;
    
    block = sbrk(total_size);
    if (block == (void*) -1) {
        pthread_mutex_unlock(&global_malloc_lock);
        return NULL;
    }

    header = block;
    header->s.size = size;
    header->s.is_free = 0;
    header->s.next = NULL;

    if (!head)
        head = header;
    if (tail)
        tail->s.next = header;
    tail = header;

    pthread_mutex_unlock(&global_malloc_lock);
    return (void*)(header + 1);
}

void* calloc(size_t num, size_t nsize) {
    size_t size;
    void* block;

    if (!num || !nsize)
        return NULL;
    size = num * nsize;

    // check overflow //
    if (nsize != size / num)
        return NULL;

    block = malloc(size);
    if (!block)
        return NULL;
    
    memset(block, 0, size);
    return block;
}

void* realloc(void* block, size_t size) {
    header_t* header;
    void* ret;
    
    if (!block || !size)
        return malloc(size);
    
    header = (header_t*)block - 1;
    if (header->s.size >= size)
        return block;
    
    ret = malloc(size);
    if (ret) {
        memcpy(ret, block, header->s.size);
        free(block);
    }
    return ret;
}