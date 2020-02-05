#ifndef MEMDOM_LIB_H
#define MEMDOM_LIB_H

#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include "smv_lib.h"
#include "kernel_comm.h"

#define MAIN_THREAD 0

/* Permission */
#define MEMDOM_READ             0x00000001
#define MEMDOM_WRITE            0x00000002
#define MEMDOM_EXECUTE          0x00000004
#define MEMDOM_ALLOCATE         0x00000008

/* MMAP flag for memdom protected area */
#define MAP_MEMDOM      0x00800000

/* Maximum heap size a memdom can use: 1GB */
//#define MEMDOM_HEAP_SIZE 0x40000000
#define MEMDOM_HEAP_SIZE 0x1000

/* Maximum number of memdoms a thread can have: 1024*/
#define MAX_MEMDOM 1024

/* Minimum size of bytes to allocate in one chunk */
#define CHUNK_SIZE 64

//#define INTERCEPT_MALLOC
#ifdef INTERCEPT_MALLOC
#define malloc(sz) memdom_alloc(memdom_private_id(), sz)
#endif

//#define MEMDOM_BENCH

#ifdef MEMDOM_BENCH
#include "mem_benchmarking_util.h"
#endif

/* Every allocated chunk of memory has this block header to record the required
 * metadata for the allocator to free memory
 */
struct alloc_metadata {
    void *addr;
    unsigned long size;
    struct alloc_metadata *next;
};

/* Memory domain metadata structure
 * A memory domain is an anonymously mmap-ed memory area.
 * mmap() is called when memdom_alloc is called the first time for a given memdom
 * Subsequent allocation does not invoke mmap(), instead, it allocates memory from the mmaped
 * area and update related metadata fields.
 */
struct memdom_metadata_struct {
  int memdom_id;
  void *start;    // start of this memdom's addr (inclusive)
  unsigned long total_size; // the total memory size of this memdom
  struct alloc_metadata *free_list_head;
  struct alloc_metadata *free_list_tail;
  struct alloc_metadata *allocs;
  unsigned long cur_alloc;
  unsigned long peak_alloc;
#ifdef PYR_MEMDOM_BENCH
  unsigned long metadata_cur;
  unsigned long metadata_peak;
#endif
  pthread_mutex_t mlock;  // protects this memdom in sn SMP environment
};

#ifdef __cplusplus
extern "C" {
#endif

  // register new memdom with kernel
  int memdom_register_new(void);
  
  /* Create memory domain and return it to user */
  int memdom_create(void);

  /* Remove memory domain memdom from kernel */
  int memdom_kill(int memdom_id);

  /* Allocate memory region in memory domain memdom */
  void *memdom_mmap(int memdom_id,
                    unsigned long addr, unsigned long len,
                    unsigned long prot, unsigned long flags,
                    unsigned long fd, unsigned long pgoff);

  /* Allocate npages pages in memory domain memdom */
  void *memdom_alloc(int memdom_id, unsigned long nbytes);

  /* Deallocate npages pages in memory domain memdom */
  void memdom_free(void* data);

  void memdom_free_from(int memdom_id, void *data);

  /* Return privilege status of smv rib in memory domain memdom */
  unsigned long memdom_priv_get(int memdom_id, int smv_id);

  /* Add privilege of smv rib in memory domain memdom */
  int memdom_priv_add(int memdom_id, int smv_id, unsigned long privs);

  /* Delete privilege of smv rib in memory domain memdom */
  int memdom_priv_del(int memdom_id, int smv_id, unsigned long privs);

  /* Get the memdom id for global memory used by main thread */
  int memdom_main_id(void);

  /* Get the memdom id for a memory address */
  int memdom_query_id(void *obj);

  /* Get the calling thread's default memdom id */
  int memdom_private_id(void);

    /* Get the number of free bytes in a memdom */
    unsigned long memdom_get_free_bytes(int memdom_id);

#ifdef PYR_MEMDOM_BENCH
    unsigned long memdom_get_peak_metadata_alloc(int memdom_id);
#endif

#ifdef __cplusplus
}
#endif

#endif
