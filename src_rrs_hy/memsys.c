#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <iostream>
#include "externs.h"
#include "memsys.h"
#include "mcore.h"

using namespace std;

////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////

MemSys *memsys_new(uns num_threads, uns64 rh_threshold){
    MemSys *m = (MemSys *) calloc (1, sizeof (MemSys));
    m->num_threads    = num_threads;
    m->rh_threshold   = rh_threshold;

    // init main memory DRAM
    m->mainmem = dram_new(MEM_SIZE_MB*1024*1024, MEM_CHANNELS, MEM_BANKS, MEM_PAGESIZE, 
			  MEM_T_ACT, MEM_T_CAS, MEM_T_RAS, MEM_T_RP, MEM_T_BURST);
    sprintf(m->mainmem->name, "MEM");
    m->lines_in_mainmem_rbuf = MEM_PAGESIZE/LINESIZE; // static

    //-- TASK A --
    //-- TODO: Initialize Misra Gries Tracker

    //-- Think? How many trackers do you need? 
    //HINT: Look at mgries_new(..) func or memsys_print_stats func.    
    int num_mg_trackers = MEM_BANKS;

    uns act_max = (64 * 4 * 1000000) / (MEM_T_RAS + MEM_T_RP);
    uns num_entries = (act_max / (m->rh_threshold/6));

    /// Modifying the mgries tracker to use this as gct.
    m->gct = (GCT_table*)   calloc(1,sizeof(GCT_table));
    m->gct = gct_new(15360,0.4*rh_threshold);
 
    m->rct = (CraCtr*)calloc(1, sizeof(CraCtr));
    m->rct = cra_ctr_new(1920 * 1024, rh_threshold/2,0.4*rh_threshold);

    // RRS data structure: row indirection table
    uns64 rit_rows = 2*num_entries;

    m->rit = (RIT**) calloc(num_mg_trackers,sizeof(RIT*));
    for(int i=0; i<num_mg_trackers; i++){
      m->rit[i] = rit_new(rit_rows, i);
    }

    return m;
}


//////////////////////////////////////////////////////////////////////////
// NOTE: ACCESSES TO THE MEMORY USE LINEADDR OF THE CACHELINE ACCESSED
//////////////////////////////////////////////////////////////////////////

uns64 memsys_access(MemSys *m, Addr lineaddr,  uns tid, uns64 in_cycle){
  uns64 total_delay=0;
  uns64 memdelay=0;

  memdelay=memsys_dram_access(m, lineaddr, in_cycle, &(m->dram_acc_info));
  total_delay=memdelay;

  // stat collection happens below this line
  m->s_totaccess++;
  m->s_totdelaysum+=total_delay;

  
  // return delay
  return total_delay;
}



////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////

void memsys_print_stats(MemSys *m)
{
    char header[256];
    sprintf(header, "MSYS");

    uns64 avg_delay=0;
    if(m->s_totaccess){
      avg_delay=m->s_totdelaysum/m->s_totaccess;
    }

    dram_print_stats(m->mainmem);

    for(uns i=0; i< m->mainmem->num_banks; i++){
      m->s_rit_tot_mig += m->rit[i]->s_num_swaps;
    }

    printf("\n%s_TOT_ACCESS      \t : %llu",    header, m->s_totaccess);
    printf("\n%s_AVG_DELAY       \t : %llu",    header, avg_delay);
    printf("\n%s_RIT_TOT_MIGRATIONS \t : %llu",    header, m->s_rit_tot_mig);
    printf("\n");


    if(m->rct)
      cra_ctr_print_stats(m->rct);

    if(m->rit)
      for(uns i=0; i< m->mainmem->num_banks; i++)
        rit_print_stats(m->rit[i]);


}

Addr random_num(Addr min_num, Addr max_num){
  Addr result = 0, low_num = 0, hi_num = 0;

  if (min_num < max_num)
  {
    low_num = min_num;
    hi_num = max_num + 1; // include max_num in output
  } else {
    low_num = max_num + 1; // include max_num in output
    hi_num = min_num; 
  }

  srand(time(NULL));
  result = (rand() % (hi_num - low_num)) + low_num;
  return result;
}

//////////////////////////////////////////////////////////////////////////
// NOTE: ACCESSES TO THE MEMORY USE LINEADDR OF THE CACHELINE ACCESSED
//-- for input ACTinfo* act_info: if the pointer is not NULL, then *act_info is updated, else not 
////////////////////////////////////////////////////////////////////
uns64  memsys_dram_access(MemSys *m, Addr lineaddr, uns64 in_cycle, ACTinfo *act_info){
  DRAM_ReqType type=DRAM_REQ_RD;
  double burst_size=1.0; // one cache line
  uns64 delay=0;

  delay += dram_service(m->mainmem, lineaddr, type, burst_size, in_cycle, act_info);

  // parse address bits and get my bank
  uns64 mybankid, myrowbufid, mychannelid;
  dram_parseaddr(m->mainmem, lineaddr, &myrowbufid,&mybankid,&mychannelid);

  if (act_info->isACT) {
      
    Flag issue_mitigation = 0;
    uns64 gct_index = act_info->rowID >> 7; // Similar to the thing you do in rct initialize
     
    if (m->gct->entries[gct_index].count == m->gct->threshold - 1) { // do we need a valid here
    // implies transfer the control rct and initialize the counters to TG.
    // trnasfer the control to the rcc.
    // m->gct->entries[gct_index].count++;
      if (m->gct->entries[gct_index].valid == 0) { // Point is doing the init only once.
        delay+= rct_initialize(m->rct, m->mainmem, act_info->rowID, in_cycle);
        m->gct->entries[gct_index].valid = 1;
      }
      else { // Go to RCT
        // Go to RCT now for servicing stuff/ SIMILAR TO cra  
        uns64 read_ctrval;
        delay += cra_ctr_read(m->rct, m->mainmem, act_info->rowID, in_cycle, &read_ctrval);
        cra_ctr_write(m->rct, m->mainmem, act_info->rowID, in_cycle, read_ctrval);// Once you get a mitigate have to reset the value back to 0.
    
        Flag issue_rit_mitigation = rit_access(m->rit[mybankid], lineaddr/(m->mainmem->lines_in_rowbuf));
        Addr random_addr = random_num(lineaddr/(m->mainmem->lines_in_rowbuf), lineaddr/(m->mainmem->lines_in_rowbuf) + m->mainmem->lines_in_rowbuf );
        if(rit_access(m->rit[mybankid], random_addr))
          random_addr = random_num(lineaddr/(m->mainmem->lines_in_rowbuf), lineaddr/(m->mainmem->lines_in_rowbuf) + m->mainmem->lines_in_rowbuf );

        if (!issue_rit_mitigation) {
         // 2*2 times memory acccess done for row swap operation
          delay += dram_service(m->mainmem, lineaddr/(m->mainmem->lines_in_rowbuf), type, burst_size, in_cycle, act_info);
          delay += dram_service(m->mainmem, random_addr, type, burst_size, in_cycle, act_info);
          delay += rit_swap(m->rit[mybankid], lineaddr/(m->mainmem->lines_in_rowbuf), random_addr);
        }
        else 
        {
          delay += rit_drain(m->rit[mybankid]);
          delay += dram_service(m->mainmem, lineaddr/(m->mainmem->lines_in_rowbuf), type, burst_size, in_cycle, act_info);
        }
      }
    }
    else {
      m->gct->entries[gct_index].count++; // no valid is required lol      
    }
  }

  return delay;
}


////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////

