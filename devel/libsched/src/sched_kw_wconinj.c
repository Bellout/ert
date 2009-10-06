#include <stdlib.h>
#include <stdbool.h>
#include <stringlist.h>
#include <string.h>
#include <util.h>
#include <sched_kw_wconinj.h>
#include <sched_kw_untyped.h>
#include <sched_util.h>
#include <stringlist.h>



/**
   This file implements a very basic support for the WCONINJ
   keyword. It internalizes a list of well names, apart from that all
   information is just tucked into a untyped keyword.

   This means that all the functionality which is supported by the
   sched_kw_wconinj implementation is asking for well names.

   It is an independent implementation - but the original
   implementation is PURE COPY AND PASTE from the WCONPROD
   implementation.
*/
   




struct sched_kw_wconinj_struct {
  sched_kw_untyped_type * untyped_kw;
  stringlist_type       * wells;
};





static sched_kw_wconinj_type * sched_kw_wconinj_alloc_empty(bool alloc_untyped)
{
  sched_kw_wconinj_type * kw = util_malloc(sizeof * kw, __func__);
  kw->wells      = stringlist_alloc_new();
  if (alloc_untyped)
    kw->untyped_kw = sched_kw_untyped_alloc_empty("WCONINJ" ,  -1 /* -1: Variable length keyword */ );  
  else
    kw->untyped_kw = NULL;
  return kw;
}



void sched_kw_wconinj_free(sched_kw_wconinj_type * kw)
{
  stringlist_free(kw->wells);
  sched_kw_untyped_free(kw->untyped_kw);
}


static void sched_kw_wconinj_add_well(sched_kw_wconinj_type * kw , const char * well) {
  stringlist_append_copy(kw->wells , well);
}





sched_kw_wconinj_type * sched_kw_wconinj_alloc(const stringlist_type * tokens , int * token_index ) {
  sched_kw_wconinj_type * kw = sched_kw_wconinj_alloc_empty( true );
  int eokw                    = false;
  do {
    stringlist_type * line_tokens = sched_util_alloc_line_tokens( tokens , false , 0 , token_index );
    if (line_tokens == NULL)
      eokw = true;
    else {
      char * well = util_alloc_dequoted_copy( stringlist_iget( line_tokens , 0 ) );
      sched_kw_wconinj_add_well(kw , well);
      sched_kw_untyped_add_tokens(kw->untyped_kw , line_tokens);
      stringlist_free( line_tokens );
      free( well );
    } 
    
  } while (!eokw);
  return kw;
}


void sched_kw_wconinj_fprintf(const sched_kw_wconinj_type * kw , FILE * stream) {
  sched_kw_untyped_fprintf( kw->untyped_kw , stream );
}


char ** sched_kw_wconinj_alloc_wells_copy( const sched_kw_wconinj_type * kw , int * num_wells) {
  *num_wells = stringlist_get_size( kw->wells );
  return stringlist_alloc_char_copy( kw->wells );
}

sched_kw_wconinj_type * sched_kw_wconinj_copyc(const sched_kw_wconinj_type * kw) {
  util_abort("%s: not implemented ... \n",__func__);
  return NULL;
}



/*****************************************************************/

KW_IMPL(wconinj)
     
