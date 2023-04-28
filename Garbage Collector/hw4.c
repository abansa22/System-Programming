#include "memlib.h"
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "mm.h"
#include "hw4.h"

struct node * root_table[ROOT_NR] = {};

int in_use(unsigned int * hdrp) {
  return (*hdrp) & 0x1u;
}

// the three lowest bits are not part of the size
// 0x1 is already used for free blocks; we use 0x2
#define MARK_BIT (((unsigned int)0x2))

// marking related operations
int is_marked(unsigned int * hdrp) {
  return ((*hdrp) & MARK_BIT) >> 1; // return 1 or 0
}

void mark(unsigned int * hdrp) {
  (*hdrp) |= MARK_BIT;
}

void unmark(unsigned int * hdrp) {
  (*hdrp) &= ~MARK_BIT;
}

// same to GET_SIZE in mm.c
unsigned int block_size(unsigned int * hdrp) {
  return (*hdrp) & (~7u);
}

void* next_hdrp(unsigned int * hdrp) {
  const unsigned int size = block_size(hdrp);
  if (size == 0) {
    fprintf(stderr,"%s Panic, chunk is of zero size.\n", __func__);
    exit(-1);
  }
  hdrp = (unsigned int *)(((char *)hdrp) + size);
  return block_size(hdrp) ? hdrp : NULL;
}

void heap_stat(const char * msg)
{
  void *hdrp = mm_first_hdr();
  size_t nr_inuse = 0;
  size_t sz_inuse = 0;
  size_t nr_free = 0;
  size_t sz_free = 0;

  while (hdrp && block_size(hdrp)) {
    if (in_use(hdrp)) {
      nr_inuse++;
      sz_inuse += block_size(hdrp);
    } else {
      nr_free++;
      sz_free += block_size(hdrp);
    }
    if (is_marked(hdrp))
      printf("%s WARNING: found a mark\n", __func__);
    hdrp = next_hdrp(hdrp);
  }

  printf("%s: %s: heapsize %zu  inuse %zu %zu  free %zu %zu\n", __func__, msg,
      mem_heapsize(), nr_inuse, sz_inuse, nr_free, sz_free);
}

// ========
// add helper functions here if any

void gc_mark(){
  void* bp = 0;
  void* start = mm_first_hdr();
  void* end = (char*)mm_first_hdr() + mem_heapsize();
  for(int i = 0; i < ROOT_NR; i++) {
    bp = root_table[i];
    while ((bp >= start) && (bp <= end)){
      void * hdr = ((char *) bp) - 4;
      mark(hdr);
      bp = ((struct node*)bp)->next;
    }
  }
}
void gc_sweep(){
  void* hdrp = mm_first_hdr();
  void* bp;
  while(hdrp && block_size(hdrp)) {
    if (in_use(hdrp) && !is_marked(hdrp)) {
      bp = ((char*)hdrp) + 4;
      mm_free(bp);
    }
    unmark(hdrp);
    hdrp = next_hdrp(hdrp);
  }
}
// garbage collection: Mark and Sweep
void gc() {
  gc_mark();
  gc_sweep();
}
