#include <stdio.h>
#include <stdlib.h>

#include "rqa.h"

///////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////

Rqa *rqa_new(uns num_rows, uns64 threshold) {
  Rqa *r = (Rqa *) calloc(1, sizeof(Rqa));
  r->num_rows = num_rows;
  r->entries = (uns64 *) calloc (r->num_rows, sizeof(uns64));
  r->head = 0;
  r->tail = 0;
  r->prev_head = 0;
  r->next_free_qr = 0;
  r->threshold = threshold;
  r->last_reset = 0;

  // Initialize statistics
  r->s_num_accesses = 0;
  r->s_num_migrations = 0;
  r->s_num_drains = 0;

  return r;
}

void rqa_reset(Rqa *rqa) {
  rqa->prev_head = rqa->next_free_qr;
}

uns64 rqa_check_delay() {
  // add a delay for 2 rb conflicts
  return 22;
}

void rqa_remove_row(Addr rqa_addr, PtrTable *rpt, PtrTable *fpt) {
  uns64 removed_row = rpt->entries[rqa_addr].addr;
  rpt->entries[rqa_addr].valid = FALSE;
  fpt->entries[removed_row].valid = FALSE;
}

void rqa_drain_row(Rqa *rqa, PtrTable *rpt, PtrTable *fpt) {
  if((rqa->tail != rqa->prev_head) && (rpt->entries[rqa->tail].valid)) {
    rqa->s_num_drains++;
    rqa_remove_row(rqa->tail, rpt, fpt);
    rqa->tail = (rqa->tail + 1)%rqa->num_rows;
  }
}

uns64 rqa_migrate(Rqa *rqa, Addr addr, PtrTable *fpt, PtrTable *rpt) {

  rqa->s_num_migrations++;
  uns64 delay = 0;
  Addr new_row;
  Flag origRowQuarantine = FALSE;
  if(fpt->entries[addr].valid) {
    new_row = fpt->entries[addr].addr;
    origRowQuarantine = TRUE;
  }

  fpt->entries[addr].valid = TRUE;
  fpt->entries[addr].addr = rqa->next_free_qr;

  delay += 10;

  if(rpt->entries[rqa->next_free_qr].valid) {
    Addr removed_row = rpt->entries[rqa->next_free_qr].addr;
    rqa_remove_row(rqa->next_free_qr, rpt, fpt);
  }

  rpt->entries[rqa->next_free_qr].valid = TRUE;
  rpt->entries[rqa->next_free_qr].addr = addr;

  if(origRowQuarantine) {
    rpt->entries[new_row].valid = FALSE;
  }

  if((rqa->next_free_qr == rqa->tail) && (rqa->next_free_qr != rqa->prev_head)) {
    rqa->tail = (rqa->tail + 1)%rqa->num_rows;
  }

  rqa->next_free_qr = (rqa->next_free_qr + 1)%rqa->num_rows;

  return delay;
}

void rqa_print_stats(Rqa *rqa) {
  printf("\nRQA");
  printf("\nNUM ACCESS \t : %llu", rqa->s_num_accesses);
  printf("\nNUM MIGRATIONS \t : %llu", rqa->s_num_migrations);
  printf("\nNUM DRAINS \t : %llu", rqa->s_num_drains);
  printf("\n");
}

