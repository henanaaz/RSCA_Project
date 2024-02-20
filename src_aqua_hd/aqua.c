#include <stdio.h>
#include <stdlib.h>
#include "aqua.h"

///////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////

RQA *rqa_new(uns num_rows, uns64 threshold) {
  RQA *r = (RQA*) calloc(1, sizeof(RQA));
  r->num_rows = num_rows;
  r->entries = (uns64 *) calloc (r->num_rows, sizeof(uns64));
  r->head_ptr = 0;
  r->tail_ptr = 0;
  r->reset = 0;
  r->prev_head = 0;
  r->next_free_qr = 0;
  r->threshold = threshold;

  r->s_num_accesses = 0;
  r->s_num_migrations = 0;
  r->s_num_drains = 0;

  return r;
}

////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////

PtrT* ptrtables_new(uns64 entries) {
  PtrT* pt = (PtrT*) calloc (1, sizeof (PtrT));
  pt->entries  = (PtrT_Entry *) calloc (entries, sizeof(PtrT_Entry));
  pt->num_rows = entries;
  return pt;
}

////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////

void rqa_reset(RQA *rqa) {
  rqa->prev_head = rqa->next_free_qr;
}

////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////

uns64 rqa_migrate(RQA* rqa, Addr addr, PtrT* fpt, PtrT* rpt) {
    uns64 delay = 0;
    Addr new_row;
    Flag row_quarantined = FALSE;

    if (fpt->entries[addr].valid) {
        new_row = fpt->entries[addr].addr;
        row_quarantined = TRUE;
    }

    // Migrate the row
    fpt->entries[addr].valid = TRUE;
    fpt->entries[addr].addr = rqa->next_free_qr;
    delay += 10;

    // Check if the next free row in rpt is valid, migrate if needed
    if (rpt->entries[rqa->next_free_qr].valid) {
        uns64 migrated_row = rpt->entries[rqa->next_free_qr].addr;
        rpt->entries[rqa->next_free_qr].valid = FALSE;
        fpt->entries[migrated_row].valid = FALSE;
    }

    // Update rpt with the migrated row
    rpt->entries[rqa->next_free_qr].valid = TRUE;
    rpt->entries[rqa->next_free_qr].addr = addr;

    // Reset the old row in rpt if it was originally quarantined
    if (row_quarantined) {
        rpt->entries[new_row].valid = FALSE;
    }

    // Update tail_ptr if needed
    if (rqa->next_free_qr != rqa->prev_head) {
      if(rqa->next_free_qr == rqa->tail_ptr) {
        rqa->tail_ptr = (rqa->tail_ptr + 1) % rqa->num_rows;
      }
    }

    // Update next_free_qr
    rqa->next_free_qr = (rqa->next_free_qr + 1) % rqa->num_rows;

    return delay;
}

////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////

Addr rpt_access(PtrT* pt, Addr rowAddr) {
  return ((pt->entries[rowAddr].valid) && pt->entries[rowAddr].addr );
}

////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////

void rqa_print_stats(RQA* rqa) {
  printf("\nRQA_NUM_ACCESS \t : %llu", rqa->s_num_accesses);
  printf("\nRQA_NUM_MIGRATIONS \t : %llu", rqa->s_num_migrations);
  printf("\nRQA_NUM DRAINS \t : %llu", rqa->s_num_drains);
  printf("\n");
}

////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////