#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include "gct.h"

////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////



GCT_table* gct_new(uns num_entries, uns threshold) { // Dont think so this is bank ID.
    GCT_table* g = (GCT_table*)calloc(1, sizeof(GCT_table));
    g->entries = (GCT_entry*)calloc(num_entries, sizeof(GCT_entry));
    g->num_entries = num_entries;
    g->threshold = threshold; // the theshold you send to gct is TG not and not T (512) for out case.

    return g;
}
////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////



void    gct_reset(GCT_table* g) { // Need to imlement rcc and rct reset too.
    uns ii;

    //------ update global stats ------
    g->s_num_reset++;

    //----- reset the structures ------

    for (ii = 0; ii < g->num_entries; ii++) {
        g->entries[ii].valid = 0;
        g->entries[ii].count = 0;
    }
}
////////////////////////////////////////////////////////////////////
// The rowAddr field is the row to be updated in the tracker
// returns TRUE if mitigation must be issued for the row
////////////////////////////////////////////////////////////////////


Flag gct_access(GCT_table *g, Addr gct_index) {
    Flag retval = FALSE;
    // Lets have a valid later
    g->entries[gct_index].count++; // Might aswell do this in the memsys call only its okay. wiritin it here for a claruty
    if (g->entries[gct_index].count == g->threshold) {
        retval = TRUE;
    }
    return retval;

}

////////////////////////////////////////////////////////////////////
// print stats for the tracker
// DO NOT CHANGE THIS FUNCTION
////////////////////////////////////////////////////////////////////


void    gct_print_stats(GCT_table* m) {
    char header[256];

    printf("\n%s_NUM_RESET      \t : %llu", header, m->s_num_reset);

    printf("\n");
}
