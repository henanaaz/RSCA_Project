#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include "mgries.h"
#include<iostream>
using namespace std;

////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////

MGries* mgries_new(uns num_entries, uns threshold, Addr bankID) {
  MGries* m = (MGries*)calloc(1, sizeof(MGries));
  m->entries = (MGries_Entry*)calloc(num_entries, sizeof(MGries_Entry));
  m->threshold = threshold; //to accomodate E = ACTmax/Trrs
  m->num_entries = num_entries;
  m->bankID = bankID;
  return m;
}


////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////

void    mgries_reset(MGries* m) {
  uns ii;
  //------ update global stats ------
  m->s_num_reset++;
  m->s_glob_spill_count += m->spill_count;

  //----- reset the structures ------
  m->spill_count = 0;
  
  for (ii = 0; ii < m->num_entries; ii++) {
    m->entries[ii].valid = FALSE;
    m->entries[ii].addr = 0;
    m->entries[ii].count = 0;
  }  
}

////////////////////////////////////////////////////////////////////
// The rowAddr field is the row to be updated in the tracker
// returns TRUE if mitigation must be issued for the row
////////////////////////////////////////////////////////////////////

Flag  mgries_access(MGries* m, Addr rowAddr) {
  Flag retval = FALSE;
  m->s_num_access++;
  Flag entry = FALSE;

  //---- TODO: Access the tracker and install entry (update stats) if needed
  int start_search_spill_over_count = 0;

  for (uns i = 0; i < m->num_entries;i++) {
    if (m->entries[i].valid == 1 && m->entries[i].addr == rowAddr) {
      m->entries[i].count++;
      entry = TRUE;
      break;
    }
  }

  // entries installed or equivalent to spillover count
  if(!entry) {
    // install the entry
    for (uns i = 0; i < m->num_entries;i++) {
      if (m->entries[i].valid == 0) {
        m->entries[i].addr = rowAddr;
        m->entries[i].valid = 1;
        m->entries[i].count++;
        entry = TRUE;
        m->s_num_install++;
        break;
      }
    }
    
    for(uns j = 0; j < m->num_entries; j++) {
      if(!entry && (m->entries[j].count == m->spill_count) && m->entries[j].valid) {
        // if not found then new entry id increemented
        m->entries[j].addr = rowAddr;
        m->entries[j].count++;
        m->s_num_install++;
        start_search_spill_over_count = 1;

        break;
      }
    }

    // increment spill counter if no entry equivalent found
    if (!entry && start_search_spill_over_count == 0) {
      m->spill_count++;
    }
  }

  //Mitigative action
  for(uns  i= 0; i < m->num_entries; i++) {
    if((m->entries[i].valid) && (m->entries[i].addr == rowAddr )) {
      if((m->entries[i].count%m->threshold == 0) && (m->entries[i].count >= m->threshold)){
        retval = 1;
        break;
      }
    }
  }

  if (retval == 1) {
    m->s_mitigations++;
  }
  
  return retval;



}

////////////////////////////////////////////////////////////////////
// print stats for the tracker
// DO NOT CHANGE THIS FUNCTION
////////////////////////////////////////////////////////////////////

void    mgries_print_stats(MGries *m){
    char header[256];
    sprintf(header, "MGRIES-%llu",m->bankID);

    printf("\n%s_NUM_RESET      \t : %llu",    header, m->s_num_reset);
    printf("\n%s_GLOB_SPILL_CT  \t : %llu",    header, m->s_glob_spill_count);
    printf("\n%s_NUM_ACCESS     \t : %llu",    header, m->s_num_access);
    printf("\n%s_NUM_INSTALL    \t : %llu",    header, m->s_num_install);
    printf("\n%s_NUM_MITIGATE   \t : %llu",    header, m->s_mitigations);

    printf("\n"); 
}
