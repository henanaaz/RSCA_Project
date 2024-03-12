#include <stdio.h>
#include <stdlib.h>
#include "rrs.h"

////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////

RIT* rit_new(uns num_entries, Addr bankID) {
  RIT* rit = (RIT*)calloc(1, sizeof(RIT));
  rit->entries = (RIT_Entry*)calloc(num_entries, sizeof(RIT_Entry));
  rit->num_entries = num_entries;
  rit->bankID = bankID;
  rit->reset = 0;
  return rit;
}

Flag rit_access(RIT *rit, Addr source_row){

  Flag retval = FALSE;
  rit->s_num_accesses++;

  for (uns i = 0; i < rit->num_entries;i++) {
    if (rit->entries[i].valid == 1) {
        if ( rit->entries[i].source_x == source_row || rit->entries[i].source_y == source_row) {
            return TRUE;
        }
    }
  }
    rit->s_num_accesses++;
    return retval; // Source row not found in RIT
}

// Perform row swapping in the RIT
uns64 rit_swap(RIT *rit, Addr source_row, Addr dest_row) {
    int delay = 0;

  for (uns i = 0; i < rit->num_entries;i++) {
    if (rit->entries[i].valid == 0) {
        delay += rit_drain(rit);
        rit->entries[i].source_x = source_row;
        rit->entries[i].source_y = dest_row;
        rit->entries[i].dest_x = dest_row;
        rit->entries[i].dest_y = source_row;
        rit->entries[i].valid = TRUE;
        rit->s_num_swaps = rit->s_num_swaps + 2;
        delay += 4*10; // to account approximation for swap buffers
        break;
    }
  }

    return delay;
}

// Reset the RIT
void rit_reset(RIT *rit) {
  for (uns i = 0; i < rit->num_entries;i++) {
    rit->entries[i].valid = FALSE;
  }
  rit->s_num_reset++;
}

// Perform lazy drain operation on the RIT
uns64 rit_drain(RIT *rit) {
    // Implementation of lazy drain operation goes here
    int delay = 4*10; //to accomodate unswaps of 2 entries
    rit->s_num_drains = rit->s_num_drains + 2;
        // Iterate through the RIT entries
    /*for (uns64 i = 0; i < rit->num_entries; i++) {
        // Check if the entry is not locked
        if (!rit->entries[i].valid) {
            // Remove the entry by shifting subsequent entries
            for (uns64 j = i; j < rit->num_entries - 1; j++) 
            {
                rit->entries[j] = rit->entries[j + 1];
            }
        }
    }*/

    return delay;
}


void rit_print_stats(RIT* rit) {
  printf("\nRIT_NUM_ACCESS \t : %llu", rit->s_num_accesses);
  printf("\nRIT_NUM_MIGRATIONS \t : %llu", rit->s_num_swaps);
  printf("\nRIT_NUM DRAINS \t : %llu", rit->s_num_drains);
  printf("\n");
}