#ifndef MGRIES_H
#define MGRIES_H

#include "global_types.h"



typedef struct GCT_entry GCT_entry;
typedef struct GCT_table GCT_table;

///////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////

struct MGries_Entry {
    Flag    valid;
    Addr    addr;
    uns     count;
};

struct GCT_entry {
    Flag valid;
    uns count;
};

struct GCT_table {
    GCT_entry* entries;
    uns num_entries;
    uns threshold; // This is TG trheshodl.
    uns64 s_num_reset;
};
//RCT will be basically cra only 


///////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////


GCT_table* gct_new(uns entries, uns threshold);

Flag    gct_access(GCT_table* g, Addr gct_index);

void    gct_reset(GCT_table* m);

void    gct_print_stats(GCT_table* g);
///////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////

#endif // MGRIES_H
