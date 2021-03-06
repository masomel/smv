/* Tests the SMV memory domain faults. Adapted from the original
 * SMV userland testcases.
 *
 *@author Marcela S. Melara
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <smv_lib.h>
#include <memdom_lib.h>
#include <pthread.h>

#define MAIN_THREAD 0

static void *memdom_read_trigger(void *buf) {
  printf("reading buffer: %s\n", (char *)buf);
  return NULL;
}

static void *memdom_write_trigger(char *buf) {
  buf[0] = 'b';
  return NULL;
}

int main(){
  
    printf("-- Test: thread memdom write fault... \n");
    int memdom_id = -1;
    int smv_id = -1;
    int err = 0;
    pthread_t tid1, tid2;
    char *str;

    smv_main_init(1);

    memdom_id = memdom_create();
    if (memdom_id == -1) {
        printf("memdom_create returned %d\n", memdom_id);
        err = -1;
    }

    smv_id = smv_create();
    if (smv_id == -1) {
        printf("memdom_create returned %d\n", memdom_id);
        err = -1;
	goto out;
    }

    // add this memory domain to the main thread SMV
    smv_join_domain(memdom_id, MAIN_THREAD);
    memdom_priv_add(memdom_id, MAIN_THREAD, MEMDOM_WRITE | MEMDOM_READ);

    str = memdom_alloc(memdom_id, 6*sizeof(char));
    sprintf(str, "hello");

    smv_join_domain(memdom_id, smv_id);
    memdom_priv_add(memdom_id, smv_id, MEMDOM_READ);
    
    // first read domain
    err = smvthread_create(smv_id, &tid1, memdom_read_trigger, str);
    if (err == -1) {
      printf("smvthread_create returned %d\n", err);
    }
    pthread_join(tid1, NULL);	
    
    // trigger memdom write segfault
    err = smvthread_create(smv_id, &tid2, memdom_write_trigger, str);
    if (err == -1) {
      printf("smvthread_create returned %d\n", err);
    }
    pthread_join(tid2, NULL);

    printf("after write attempt: %s\n", str);
    memdom_free(str);
    
 out:
    if (err == -1) {
      printf("failed\n");
    }

    return 0;
}
