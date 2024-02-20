#ifndef MEMSYS_H
#define MEMSYS_H


#include "global_types.h"
#include "mcache.h"
#include "dram.h"
#include "mgries.h"
#include "cra_ctr.h"
#include "rrs.h"

#define MEMSYS_MAX_THREADS 16
typedef struct MemSys MemSys;

//////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////


struct MemSys {
  uns       num_threads;  
  DRAM     *mainmem;
  uns64     lines_in_mainmem_rbuf;

  //---- Rowhammer related -----
  uns64    rh_threshold;
  ACTinfo  dram_acc_info;
  MGries   **mgries_t;
  CraCtr    *cra_t;
  uns64     s_tot_mitigate;
  //----------------------------

  // ----- RRS -------
  RIT	    *rit;

  uns64     s_totaccess;
  uns64     s_totdelaysum;
};



//////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

MemSys *memsys_new(uns num_threads, uns64 rh_threshold);
uns64   memsys_access(MemSys *m, Addr lineaddr, uns tid, uns64 in_cycle);
void    memsys_print_stats(MemSys *m);
uns64   memsys_dram_access(MemSys *m, Addr lineaddr, uns64 in_cycle, ACTinfo *act_info);


//////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

#endif // MEMSYS_H
