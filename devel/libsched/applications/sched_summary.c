#include <util.h>
#include <stdlib.h>
#include <sched_file.h>
#include <sched_history.h>
#include <ecl_util.h>
#include <time.h>
#include <stdio.h>



int main (int argc , char ** argv) {
  time_t start_time;
  if (argc < 3) 
    util_exit("usage: ECLIPSE.DATA SCHEDULE_FILE   <key1>   <key2>   <key3>  ...\n");
  {
    const char * data_file       = argv[1];
    const char * schedule_file   = argv[2];
    start_time = ecl_util_get_start_date( data_file );
    {
      sched_history_type * sched_history = sched_history_alloc( ":" );
      sched_file_type * sched_file = sched_file_alloc(start_time);
      sched_file_parse(sched_file , schedule_file);
      sched_history_update( sched_history , sched_file );

      {
        stringlist_type * key_list = stringlist_alloc_new();
        
        for (int iarg=3; iarg < argc; iarg++) {
          if( sched_history_has_key( sched_history , argv[iarg] ))
            stringlist_append_ref( key_list , argv[iarg]);
          else
            fprintf(stderr,"** Warning the SCHEDULE file does not contain the key: %s \n",argv[iarg]);
        }
        
        sched_history_fprintf( sched_history , key_list , stdout );
        stringlist_free( key_list ) ;
      }
    }
  }
}
