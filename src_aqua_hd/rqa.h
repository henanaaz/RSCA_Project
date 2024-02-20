#ifndef RQA_H
#define RQA_H

#include "global_types.h"
#include "pointer_tables.h"
#include "dram.h"

typedef struct Rqa Rqa;

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////


struct Rqa{
  uns64		num_rows;
  Addr		*entries;  // points to the start of the rqa data
  uns64		head;      // index of the next available row in the rqa
  uns64		tail;      // i
  uns64		prev_head;
  uns64		next_free_qr;
  uns64		threshold;
  uns64		last_reset;

  // -----Statistics------
  uns64		s_num_accesses;
  uns64		s_num_migrations;
  uns64		s_num_drains;
};

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

Rqa *rqa_new(uns num_rows, uns64 threshold);
void rqa_reset(Rqa *rqa);
uns64 rqa_check_delay();
void rqa_remove_row(Addr rqa_addr, PtrTable *rpt, PtrTable *fpt);
void rqa_drain_row(Rqa *rqa, PtrTable *rpt, PtrTable *fpt);
uns64 rqa_access(Rqa *rqa, uns64 in_cycle);
uns64 rqa_migrate(Rqa *rqa, Addr addr, PtrTable *fpt, PtrTable *rpt);
void rqa_print_stats(Rqa *rqa);

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

#endif // RQA_H
