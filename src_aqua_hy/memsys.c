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
    m->rh_threshold   = rh_threshold/2;

    // init main memory DRAM
    m->mainmem = dram_new(MEM_SIZE_MB*1024*1024, MEM_CHANNELS, MEM_BANKS, MEM_PAGESIZE, 
			  MEM_T_ACT, MEM_T_CAS, MEM_T_RAS, MEM_T_RP, MEM_T_BURST);
    sprintf(m->mainmem->name, "MEM");
    m->lines_in_mainmem_rbuf = MEM_PAGESIZE/LINESIZE; // static

    m->gct = (GCT_table*)   calloc(1,sizeof(GCT_table));
    m->gct = gct_new(15360,0.4*rh_threshold);
 
    m->rct = (CraCtr*)calloc(1, sizeof(CraCtr));
    m->rct = cra_ctr_new(1920 * 1024, rh_threshold/2,0.4*rh_threshold);
    

    // AQUA data structures: Quarantine area, forward and reverse pointer tables
    uns64 rqa_rows =  (2 * 1024* 1000) / ((0.022*rh_threshold) + 18.4); // 50073.3496
    m->rqa = rqa_new(rqa_rows, rh_threshold/2);
    uns64 fpt_rows = 40 * rqa_rows; //over-provisioned collision avoidance table or 2503650 at 1k/2 = 512 effective threshold
    m->fpt = ptrtables_new(fpt_rows);
    uns64 rpt_rows = rqa_rows; // 1 for every rqa row 
    m->rpt = ptrtables_new(rpt_rows);
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

    printf("\n%s_TOT_ACCESS      \t : %llu",    header, m->s_totaccess);
    printf("\n%s_AVG_DELAY       \t : %llu",    header, avg_delay);
    printf("\n%s_RH_TOT_MITIGATE \t : %llu",    header, m->s_tot_mitigate);
    printf("\n");

    if(m->rct)
      cra_ctr_print_stats(m->rct);

    if(m->rqa)
      rqa_print_stats(m->rqa);
}


//////////////////////////////////////////////////////////////////////////
// NOTE: ACCESSES TO THE MEMORY USE LINEADDR OF THE CACHELINE ACCESSED
//-- for input ACTinfo* act_info: if the pointer is not NULL, then *act_info is updated, else not 
////////////////////////////////////////////////////////////////////
uns64  memsys_dram_access(MemSys *m, Addr lineaddr, uns64 in_cycle, ACTinfo *act_info){
  DRAM_ReqType type=DRAM_REQ_RD;
  double burst_size=1.0; // one cache line
  uns64 delay=0;

  if((in_cycle - m->rqa->reset)/4000000 >= 64) {
    m->rqa->reset = in_cycle;
    rqa_reset(m->rqa);
  }

  m->rqa->s_num_accesses++;

  // parse address bits and get my bank
  uns64 mybankid, myrowbufid, mychannelid;
  dram_parseaddr(m->mainmem, lineaddr, &myrowbufid,&mybankid,&mychannelid);

  Flag issue_mitigation = 0;
  //delay += dram_service(m->mainmem, lineaddr, type, burst_size, in_cycle, act_info);
  if (act_info->isACT) {
    uns64 gct_index = act_info->rowID >> 7; //initializing gct
     
    if (m->gct->entries[gct_index].count == m->gct->threshold - 1) { // do we need a valid here
      if (m->gct->entries[gct_index].valid == 0) { // Point is doing the init only once.
        delay += rct_initialize(m->rct, m->mainmem, act_info->rowID, in_cycle);
        m->gct->entries[gct_index].valid = 1;
      } //valid end
      else 
      { // Go to RCT //not valid
        uns64 read_ctrval;
        issue_mitigation = cra_ctr_read(m->rct, m->mainmem, act_info->rowID, in_cycle, &read_ctrval);
        cra_ctr_write(m->rct, m->mainmem, act_info->rowID, in_cycle, read_ctrval);// Once you get a mitigate have to reset the value back to 0.

      }//not valid end
    } //threshold -1
    else { //threshold not reached 
      m->gct->entries[gct_index].count++;         
    }  //  threshold not reached end
  }//isACT end

  if (issue_mitigation) {
    rqa_migrate(m->rqa, lineaddr/(m->mainmem->lines_in_rowbuf), m->fpt, m->rpt);
    m->rqa->s_num_migrations = m->rqa->s_num_migrations + 2;
  }
  else {
    delay += dram_service(m->mainmem, lineaddr/(m->mainmem->lines_in_rowbuf), type, burst_size, in_cycle, act_info);
    if(m->rpt->entries[m->rqa->tail_ptr].valid && (m->rqa->tail_ptr != m->rqa->prev_head) ) {
      m->rpt->entries[m->rqa->tail_ptr].valid = (m->rqa->tail_ptr + 1)%m->rqa->num_rows;
      m->rqa->s_num_drains++;
    }
  } // not mitigation end

  return delay;
}


////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////

