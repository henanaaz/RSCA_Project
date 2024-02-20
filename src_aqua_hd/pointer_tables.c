#include <stdio.h>
#include <stdlib.h>

#include "pointer_tables.h"

////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////

PtrTable *ptrtables_new(uns64 entries) {
  PtrTable *pt = (PtrTable *) calloc (1, sizeof (PtrTable));
  pt->entries  = (PtrTable_Entry *) calloc (entries, sizeof(PtrTable_Entry));
  pt->numRows = entries;
  return pt;
}

Addr fpt_check(PtrTable *pt, Addr rowAddr) {
  uns64 index = (rowAddr/8192)%pt->numRows;
  if(pt->entries[index].valid) {
    return pt->entries[index].addr;
  }
  else
    return 0;
}

Addr rpt_access(PtrTable *pt, Addr rowAddr) {
  if(pt->entries[rowAddr].valid)
    return pt->entries[rowAddr].addr;
  else
    return 0;
}

void fpt_update(PtrTable *pt, Addr new_addr, Addr old_addr) {
  pt->entries[new_addr].valid = TRUE;
  pt->entries[new_addr].addr = old_addr;
}

void rpt_update(PtrTable *pt, Addr old_addr, Addr new_addr) {
  pt->entries[old_addr].valid = TRUE;
  pt->entries[old_addr].addr = new_addr;
}
