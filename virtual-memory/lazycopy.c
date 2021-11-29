#define _GNU_SOURCE
#include "lazycopy.h"

#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>

//struct for a list of chunks
typedef struct list {
  void* data;
  struct list *next;
} list;

void list_init(list* lst) {
  //sets a head node with value -1 that points to null
  lst->data = NULL;
  lst->next = NULL;
}

//global chunklist
list chunkList;

/**
 * This function will be called at startup so you can set up a signal handler.
 */
void signal_handler(int signal, siginfo_t* info, void* ctx);
void chunk_startup();

//Implement signal handling code here
void chunk_startup() {
  struct sigaction sa;
  memset(&sa, 0, sizeof(struct sigaction));
  sa.sa_sigaction = signal_handler;
  sa.sa_flags = SA_SIGINFO;

  if(sigaction(SIGSEGV, &sa, NULL) != 0){
    perror("sigaction failed");
    exit(2);
  }
  list_init(&chunkList);
}

void signal_handler(int signal, siginfo_t* info, void* ctx){

  //take si_addr, find what chunk its in, use that as first argument of mmap
  void* segChunk = NULL;
  list currNode = chunkList;

  while(currNode.next!=NULL){
    currNode = *currNode.next;

    //locate the chunk
    if(abs((int)info->si_addr - (int) currNode.data) < (int) CHUNKSIZE
        && abs((int)info->si_addr - (int) currNode.data)>=0){
      segChunk = currNode.data;
      break;
    }
  }

  void* tempChunk = chunk_alloc();

  // Now copy the data
  memcpy(tempChunk, segChunk, CHUNKSIZE);


  // Use mmap to make a writable mapping at the location of the chunk that was written
  void* chunk_new = mmap(segChunk, CHUNKSIZE, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_SHARED | MAP_FIXED, -1, 0);

  //check for error
  if (chunk_new == MAP_FAILED) {
    perror("mmap new failed");
    exit(EXIT_FAILURE);
  }

  //restore data from tempChunk to chunk_new
  memcpy(chunk_new, tempChunk, CHUNKSIZE);

  return;
}

/**
 * This function should return a new chunk of memory for use.
 *
 * \returns a pointer to the beginning of a 64KB chunk of memory that can be read, written, and
 * copied
 */
void* chunk_alloc() {
  // Call mmap to request a new chunk of memory. See comments below for description of arguments.
  void* result = mmap(NULL, CHUNKSIZE, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_SHARED, -1, 0);
  // Arguments:
  //   NULL: this is the address we'd like to map at. By passing null, we're asking the OS to
  //   decide. CHUNKSIZE: This is the size of the new mapping in bytes. PROT_READ | PROT_WRITE: This
  //   makes the new reading readable and writable MAP_ANONYMOUS | MAP_SHARED: This mapes a newvoid* chunk_arr[100];
  return result;
}

// Create a copy of a chunk by CHUNKSIZE
void* chunk_copy_eager(void* chunk) {
  // First, we'll allocate a new chunk to copy to
  void* new_chunk = chunk_alloc();

  // Now copy the data
  memcpy(new_chunk, chunk, CHUNKSIZE);

  // Return the new chunk
  return new_chunk;
}

/**
 * Create a copy of a chunk by copying values lazily.
 *
 * \param chunk This parameter points to the beginning of a chunk returned from chunk_alloc()
 * \returns a pointer to the beginning of a new chunk that holds a copy of the values from
 *   the original chunk.
 */
void* chunk_copy_lazy(void* chunk) {
  // Use mremap to create a duplicate mapping of the chunk passed in
  void* chunk_copy = mremap(chunk, 0, CHUNKSIZE, MREMAP_MAYMOVE);

  //check for an error
  if (chunk_copy == MAP_FAILED) {
    perror("mremap failed");
    exit(EXIT_FAILURE);
  }


  // Mark both mappings as read-only
  if (mprotect(chunk, CHUNKSIZE, PROT_READ)) {
    perror("mprotect failed");
    exit(EXIT_FAILURE);
  }
  if (mprotect(chunk_copy, CHUNKSIZE, PROT_READ)) {
    perror("mprotect failed");
    exit(EXIT_FAILURE);
  }

  // Keep some record of both lazy copies so you can make them writable later.
  // Save the contents of the chunk as linkedList

  //check to see if chunk is already in chunklist
  int add = 1;  //keep track of whether chunk is in or not
  list currNode = chunkList;
  while(currNode.next!=NULL){
    currNode = *currNode.next;
    if(currNode.data == chunk) add = 0;
  }

  //if not in list add chunk
  if(add==1){
    list* newNode = malloc(sizeof(list));
    newNode->data = chunk;
    newNode->next = NULL;
    chunkList.next = newNode;
  }

  //add chunkcopy to chunklist
  list* newNode2 = malloc(sizeof(list));
  newNode2->data = chunk_copy;
  newNode2->next = NULL;
  chunkList.next->next = newNode2;

  return chunk_copy;
}
