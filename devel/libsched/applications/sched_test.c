#include <stdbool.h>
#include <util.h>
#include <sched_file.h>
#include <history.h>
#include <sched_kw_wconinje.h>
#include <sched_history.h>
#include <well_history.h>
#include <ecl_sum.h>


int main(int argc, char **argv)
{
  time_t start_time;
  start_time = util_make_date(1,1,2000);
  start_time = util_make_date(1,1,1988);
  sched_file_type * sched_file = sched_file_parse_alloc( argv[1] , start_time);
  sched_history_type * sched_history = sched_history_alloc(":");
  sched_history_update( sched_history , sched_file );
  sched_history_install_index( sched_history );
  
  sched_history_fprintf_group_structure( sched_history , 300 );
  printf("FOPRH       %g \n",sched_history_iget( sched_history , "FOPRH"     , 139));
  //printf("GWPRH:AN    %g \n",sched_history_iget( sched_history , "GOPRH:AN"  , 139));
  //printf("WWIRH:C-1   %g \n",sched_history_iget( sched_history , "WWIRH:C-1"  , 139));
  //printf("WOPRH:C-15C %g \n",sched_history_iget( sched_history , "WOPRH:B-6A"  , 300));
  //printf("WOPRH:C-15C %g \n",sched_history_iget( sched_history , "WOPRH:B-6A"  , 400));
  
  sched_history_free( sched_history );
  sched_file_free( sched_file );
}
