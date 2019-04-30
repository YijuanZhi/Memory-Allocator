//a basic malloc/free allocator that uses the first-fit allocation scheme.

#include "my_malloc.h"
#include <assert.h>
#include <stdio.h>
#include <sys/mman.h>

// A pointer to the head of the free list.
node_t *head = NULL;

// The heap function returns the head pointer to the free list. If the heap
// has not been allocated yet (head is NULL) it will use mmap to allocate
// a page of memory from the OS and initialize the first free node.
node_t *heap() {
  if (head == NULL) {
    // This allocates the heap and initializes the head node.
    head = (node_t *)mmap(NULL, HEAP_SIZE, PROT_READ | PROT_WRITE,
                          MAP_ANON | MAP_PRIVATE, -1, 0);
    head->size = HEAP_SIZE - sizeof(node_t);
    head->next = NULL;
  }

  return head;
}

//Reallocates the heap.
void reset_heap() {
  if (head != NULL) {
    munmap(head, HEAP_SIZE);
    head = NULL;
    heap();
  }
}

// Returns a pointer to the head of the free list.
node_t *free_list() { return head; }

// Calculates the amount of free memory available in the heap.
size_t available_memory() {
  size_t n = 0;
  node_t *p = heap();
  while (p != NULL) {
    n += p->size;
    p = p->next;
  }
  return n;
}

// Returns the number of nodes on the free list.
int number_of_free_nodes() {
  int count = 0;
  node_t *p = heap();
  while (p != NULL) {
    count++;
    p = p->next;
  }
  return count;
}

// Prints the free list. Useful for debugging purposes.
void print_free_list() {
  node_t *p = heap();
  while (p != NULL) {
    printf("Free(%zd)", p->size);
    p = p->next;
    if (p != NULL) {
      printf("->");
    }
  }
  printf("\n");
}

// Finds a node on the free list that has enough available memory to
// allocate to a calling program. This function uses the "first-fit"
// algorithm to locate a free node.
//
// PARAMETERS:
// size - the number of bytes requested to allocate
//
// RETURNS:
// found - the node found on the free list with enough memory to allocate
// previous - the previous node to the found node
//
void find_free(size_t size, node_t **found, node_t **previous) {
  // TODO
  node_t* f = heap();
  node_t* p = NULL;

  if(f == NULL || size > available_memory()){
    *found = NULL;
    *previous = NULL;
    return;
  }
  while(f != NULL){
    if(((f->size) + sizeof(node_t)) >= (size + sizeof(header_t))){
      *found = f;
      *previous = p;
      return;
    }
    else{
      p = f;
      f = f->next;
    }
  }
  return;
}

// Splits a found free node to accommodate an allocation request.
//
// The job of this function is to take a given free_node found from
// `find_free` and split it according to the number of bytes to allocate.
// In doing so, it will adjust the size and next pointer of the `free_block`
// as well as the `previous` node to properly adjust the free list.
//
// PARAMETERS:
// size - the number of bytes requested to allocate
// previous - the previous node to the free block
// free_block - the node on the free list to allocate from
//
// RETURNS:
// allocated - an allocated block to be returned to the calling program
//
void split(size_t size, node_t **previous, node_t **free_block, header_t **allocated) {
  assert(*free_block != NULL);
  // TODO
  size_t requ = size + sizeof(header_t);
  //the space of free_block is just right for the requested allocated space
  size_t original_size = (*free_block)->size;
  node_t* original_next = (*free_block)->next;
  char* original = (char*) *free_block;
  // if(avai == requ){
  //   //moving the pointer of free_block and change the available free space size
  //   //changing the next of previous free block
  //   // free_block = &(*free_block)->next;
  //   // (*free_block)->size = ((*free_block)->next)->size;
  //   // (*free_block)->next = ((*free_block)->next)->next;
  //   *free_block = (*free_block)->next;
  // }
  // //the space of free_block is larger than the requested allocated space
  // else{
  *free_block =(node_t*)(((char*)(*free_block)) + requ);
  (*free_block)->size = original_size - requ;
  (*free_block)->next = original_next;
  //}
  if(*previous == NULL){
    head = *free_block;
  }else{
    (*previous)->next = *free_block;
  }
  *allocated  = (header_t*)(original);
  (*allocated)->size = size;
  (*allocated)->magic = MAGIC;
}

// Returns a pointer to a region of memory having at least the request `size`
// bytes.
//
// PARAMETERS:
// size - the number of bytes requested to allocate
//
// RETURNS:
// A void pointer to the region of allocated memory
//
void* my_malloc(size_t size) {
  // TODO
  node_t* found;
  node_t* previous;
  header_t* allocated;
  find_free(size, &found, &previous);
  if(found == NULL) return NULL;
  else{
    split(size, &previous, &found, &allocated);
    return (void*) ((char*) allocated + sizeof(header_t));
  }
}

// Merges adjacent nodes on the free list to reduce external fragmentation.
//
// This function will only coalesce nodes starting with `free_block`. It will
// not handle coalescing of previous nodes (we don't have previous pointers!).
//
// PARAMETERS:
// free_block - the starting node on the free list to coalesce
//
void coalesce(node_t *free_block) {
  // TODO
  node_t* cur = free_block;
  while(cur != NULL && cur->next != NULL){
    char* curdown = (char*) cur + cur->size + sizeof(node_t);
    char* nextup = (char*) (cur->next);
    if(curdown == nextup) {
      cur->size = cur->size + (cur->next)->size + sizeof(node_t);
      cur->next = (cur->next)->next;
    }else{
      cur = cur->next;
    }
  }
}

// Frees a given region of memory back to the free list.
//
// PARAMETERS:
// allocated - a pointer to a region of memory previously allocated by my_malloc
//
void my_free(void *allocated) {
  // TODO
  header_t* oldheader = (header_t*) ((char*)allocated - sizeof(header_t));
  assert(oldheader->magic == MAGIC);
  node_t* newfree = (node_t*)((char*) oldheader);
  newfree->size = oldheader->size + sizeof(header_t) - sizeof(node_t);
  newfree->next = head;
  head = newfree;
  coalesce(head);
}

