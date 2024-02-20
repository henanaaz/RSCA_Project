#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include "cra_ctr.h"
#include "externs.h"
////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////

CraCtr *cra_ctr_new(uns num_ctrs, uns threshold , uns threshold_g){
  CraCtr *m = (CraCtr *) calloc (1, sizeof (CraCtr));
  m->counts  = (uns64 *) calloc (num_ctrs, sizeof(uns64));
  m->num_ctrs = num_ctrs;
  m->threshold = threshold;
  m->threshold_g = threshold_g;
  assert( ((num_ctrs / NUM_CTRS_PER_CL * 64) < 1024*1024*1024) \
    && "Counter Region cannot be more than 1GB\n");
  
  //---- TODO: B2 Initialize Counter Cache for in-DRAM counters
  m->ctr_cache = ctrcache_new(CTRCACHE_SIZE_KB * 1024 / 2 / CTRCACHE_ASSOC, CTRCACHE_ASSOC);  // Size of each counter is 2 bytes 
 // printf("%d \n \n", CTRCACHE_SIZE_KB * 1024 / 2 / CTRCACHE_ASSOC);
  // use an RCC with 4K entries and 16-way associativity. Use the same replacement policy as that used in the counter cache of CRA.
  // 4k/16 sets 8K
  return m;
}

////////////////////////////////////////////////////////////////////
// The rowAddr field is the row to be updated in the tracker
// The *read_ctrval field is to be updated by the function, based on the ctr value read
// returns TRUE if mitigation must be issued for the row
////////////////////////////////////////////////////////////////////

Flag  cra_ctr_read(CraCtr *m, DRAM* d, Addr rowAddr, uns64 in_cycle, uns64* read_ctrval){
  Flag retval = FALSE;
  uns delay = 0;
  uns64 counter_index = rowAddr; // Index the cahce with the row id itself.
  bool hit = ctrcache_access(m->ctr_cache, counter_index);
  if (!hit) {
      Addr counter_address = (0x3C0000000 + 2 * counter_index) / LINESIZE;
      delay += dram_service(d, counter_address, DRAM_REQ_RD, 1.0, in_cycle, NULL); // NULL coz we dont care about the activations of the counters in DRAM.
      m->s_num_reads++;
  }
  else {
      //printf("Yay hit!!! \n");
  }
  *read_ctrval = m->counts[counter_index];


  if (((*read_ctrval+1) % (m->threshold-1) == 0) && (*read_ctrval > 1)) { // Coz we increment it in the next slide as well // PUT -1 and see.
      retval = TRUE;
  }
  if (!hit) {
      Addr return_idx = ctrcache_install(m->ctr_cache, counter_index); // Need to ask about handling writeback, is it off the critical path or do we actually care about it.
      //Handling write back
      if (return_idx != -1) {
          Addr return_addr = (0x3C0000000 + 2 * return_idx) / LINESIZE;  // Cache only gives u row id so you need to conver it whenever sending it to DRAM.
          delay += dram_service(d, return_addr, DRAM_REQ_RD, 1.0, in_cycle, NULL); // What about the other parameters?
          delay += dram_service(d, return_addr, DRAM_REQ_WB, 1.0, in_cycle, NULL);
          m->s_num_reads++;
          m->s_num_writes++;
      }
  }
  if (retval == TRUE) {
      m->s_mitigations++;
      *read_ctrval = -1;  //
      m->counts[counter_index] = -1; // Reset it back to 0;
  }

  return retval;
}

void  cra_ctr_write(CraCtr *m, DRAM* d, Addr rowAddr, uns64 in_cycle, uns64 write_ctrval){

  uns delay = 0;
  //---- TODO: B1 Calculate which cra_ctr is to be accessed

  uns64 counter_index = rowAddr;
 
  bool hit = ctrcache_access(m->ctr_cache, counter_index);
  if (hit) {
      bool dirty = ctrcache_mark_dirty(m->ctr_cache, counter_index); // Idk how i  will use this but to avoid run time issues giving it a retunr variable only in this scope.
  }
  if (!hit) {
      assert(0);
  }
  m->counts[counter_index]++;
  if (!hit) {
      assert(0);
  }

  return ;
}

void rct_initialize(CraCtr* m, DRAM* d, Addr rowAddr, uns64 in_cycle) {
    uns delay = 0;
    // 128 counters, 4 reads and 4 writes. and 1 install in the cache.
    //printf("%u \n", rowAddr);
    uns64 counter_index = rowAddr >> 7; // Get aligned row_id 
    counter_index = counter_index << 7;// Get aligned row_id 
    Addr counter_address = (0x3C0000000 + 2 * counter_index) / LINESIZE;
    //printf("shifted %u \n", counter_index);

    for (int i = counter_index; i < counter_index + 128;i++) { // Need to see if this 128 counters should be hard coded or not.
        m->counts[i] = m->threshold_g;
    }


    //Perform timing now. Weird i know. but 4 reads and 4 Wrties coz 4 cachelines. 128 counter 2 bits
    delay += dram_service(d, counter_address, DRAM_REQ_RD, 1.0, in_cycle, NULL);
    delay += dram_service(d, counter_address + 1, DRAM_REQ_RD, 1.0, in_cycle, NULL); // Should we add the delay tp the incycle here?
    delay += dram_service(d, counter_address + 2, DRAM_REQ_RD, 1.0, in_cycle, NULL);
    delay += dram_service(d, counter_address + 3, DRAM_REQ_RD, 1.0, in_cycle, NULL); 
    m->s_num_reads = m->s_num_reads + 4;
    delay += dram_service(d, counter_address, DRAM_REQ_WB, 1.0, in_cycle, NULL);
    delay += dram_service(d, counter_address + 1, DRAM_REQ_WB, 1.0, in_cycle, NULL);
    delay += dram_service(d, counter_address + 2, DRAM_REQ_WB, 1.0, in_cycle, NULL);
    delay += dram_service(d, counter_address + 3, DRAM_REQ_WB, 1.0, in_cycle, NULL);
    m->s_num_writes = m->s_num_writes + 4;
    // Install the original rowid in the cache./
   


    counter_address = rowAddr ; // sending row id to cahce install will help u have only 1 counter in the cache. or dont dividie by 64 either way its the same.
    Addr return_idx = ctrcache_install(m->ctr_cache, rowAddr); // Row id only will be returned.
    if (return_idx != -1) {
        // Paper says it has to be a RMW instead of a simple RMW
        Addr return_addr =(0x3C0000000 + 2 * return_idx)/ LINESIZE;  // Cache only gives u row id so you need to conver it whenever sending it to DRAM.
        delay += dram_service(d, return_addr, DRAM_REQ_RD, 1.0, in_cycle, NULL); // What about the other parameters?
        delay += dram_service(d, return_addr, DRAM_REQ_WB, 1.0, in_cycle, NULL);
        m->s_num_reads++;
        m->s_num_writes++;
    }
}



void rcc_flush(CraCtr* m) {
    for (int i = 0; i < m->ctr_cache->sets * m->ctr_cache->assocs; i++) {
        m->ctr_cache->entries[i].valid = 0;
    }
}
////////////////////////////////////////////////////////////////////
// stats for tracker printed here
// DO NOT MODIFY THIS FUNCTION
////////////////////////////////////////////////////////////////////

void    cra_ctr_print_stats(CraCtr *m){
    char header[256];
    sprintf(header, "CRA");
    printf("\n%s_NUM_READS     \t : %llu",     header, m->s_num_reads);
    printf("\n%s_NUM_WRITES    \t : %llu",     header, m->s_num_writes);
    printf("\n%s_NUM_MITIGATE   \t : %llu",    header, m->s_mitigations);
    printf("\n"); 
}
