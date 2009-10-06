#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <double_vector.h>
#include <sched_util.h>
#include <util.h>
#include <sched_kw_tstep.h>
#include <sched_macros.h>



struct sched_kw_tstep_struct {
  double_vector_type * tstep_list;
}; 



/*****************************************************************/


static void sched_kw_tstep_add_tstep( sched_kw_tstep_type * kw, double tstep ) {
  double_vector_append( kw->tstep_list , tstep );
}


static void sched_kw_tstep_add_tstep_string( sched_kw_tstep_type * kw, const char * tstep_string) {
  double tstep;
  if (util_sscanf_double( tstep_string , &tstep ))
    sched_kw_tstep_add_tstep(kw , tstep );
  else
    util_abort("%s: failed to parse:%s as a floating point number \n",__func__ , tstep_string);
}






static sched_kw_tstep_type * sched_kw_tstep_alloc_empty(){
  sched_kw_tstep_type *tstep = util_malloc(sizeof * tstep , __func__ );
  tstep->tstep_list          = double_vector_alloc(0 , 0);
  return tstep;
}



/*****************************************************************/

sched_kw_tstep_type * sched_kw_tstep_alloc(const stringlist_type * tokens , int * token_index ) {
  sched_kw_tstep_type * kw = sched_kw_tstep_alloc_empty();
  int eokw                    = false;
  do {
    stringlist_type * line_tokens = sched_util_alloc_line_tokens( tokens , false , 0 , token_index );
    if (line_tokens == NULL)
      eokw = true;
    else {
      int i;
      for (i=0; i < stringlist_get_size( line_tokens ); i++)
        sched_kw_tstep_add_tstep_string( kw , stringlist_iget( line_tokens , i ));
      stringlist_free( line_tokens );
    } 
  } while (!eokw);
  return kw;
}


void sched_kw_tstep_fprintf(const sched_kw_tstep_type *kw , FILE *stream) {
  fprintf(stream,"TSTEP\n  ");
  {
    int i;
    for (i=0; i < double_vector_size( kw->tstep_list ); i++)
      fprintf(stream, "%7.3f", double_vector_iget( kw->tstep_list , i));
  }
  fprintf(stream , " /\n\n");
}



void sched_kw_tstep_free(sched_kw_tstep_type * kw) {
  double_vector_free(kw->tstep_list);
  free(kw);
}



int sched_kw_tstep_get_size(const sched_kw_tstep_type * kw)
{
  return double_vector_size(kw->tstep_list);
}



sched_kw_tstep_type * sched_kw_tstep_alloc_from_double(double step)
{
  sched_kw_tstep_type * kw = sched_kw_tstep_alloc_empty();
  double_vector_append( kw->tstep_list , step );
  return kw;
}

sched_kw_tstep_type * sched_kw_tstep_copyc(const sched_kw_tstep_type * kw) {
  util_abort("%s: not implemented ... \n",__func__);
  return NULL;
}



double sched_kw_tstep_iget_step(const sched_kw_tstep_type * kw, int i)
{
  return double_vector_iget( kw->tstep_list , i );
}



double sched_kw_tstep_get_step(const sched_kw_tstep_type * kw)
{
  if(sched_kw_tstep_get_size(kw) > 1)
  {
    util_abort("%s: Internal error - must use sched_kw_tstep_iget_step instead - aborting\n", __func__);
  }

  return sched_kw_tstep_iget_step(kw, 0);
}



time_t sched_kw_tstep_get_new_time(const sched_kw_tstep_type *kw, time_t curr_time)
{
  double step_days = sched_kw_tstep_iget_step(kw , 0);
  time_t new_time  = curr_time;
  util_inplace_forward_days(&new_time, step_days);
  return new_time;
}


/*****************************************************************/

KW_IMPL(tstep)


