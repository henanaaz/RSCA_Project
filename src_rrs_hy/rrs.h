#ifndef RRS_H
#define RRS_H

#include "global_types.h"
#include "dram.h"

typedef struct RIT_Entry RIT_Entry;
typedef struct RIT RIT;

///////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////

struct RIT_Entry {
    Flag    valid;
    Addr    source_x;
    Addr    source_y;
    Addr    dest_x;
    Addr    dest_y;
};


struct RIT{
  RIT_Entry *entries;
  uns           num_entries;
  Addr          bankID;

  uns64 reset;
  uns64         s_num_reset;        //-- how many times was the rit reset

  //---- Update below statistics in rit_access() ----
  uns64         s_num_accesses;  //-- how many times was the tracker called
  uns64         s_num_install; //-- how many times did the tracker install rowIDs 
  uns64         s_num_swaps; //-- how many times did the tracker issue mitigation
  uns64         s_num_drains;

};

// Function prototypes
RIT *rit_new(uns num_rows, uns64 threshold);            // Create a new RIT
Flag rit_access(RIT *rit, Addr source_row);
uns64 rit_swap(RIT *rit, Addr source_row, Addr dest_row); // Perform row swapping
void rit_reset(RIT *rit);              // Reset the RIT
uns64 rit_drain(RIT *rit);              // Perform lazy drain operation

void rit_print_stats(RIT* rit);

#endif /* RRS_H */
