#ifndef __SCHED_KW_WCONINJE_H__
#define __SCHED_KW_WCONINJE_H__


#ifdef __cplusplus
extern "C" {
#endif
#include <sched_macros.h>
#include <sched_types.h>
#include <stringlist.h>
#include <stdbool.h>
#include <buffer.h>
#include <time_t_vector.h>

typedef struct sched_kw_wconinje_struct sched_kw_wconinje_type;
typedef struct wconinje_state_struct    wconinje_state_type;


sched_phase_enum         sched_kw_wconinje_get_phase( const sched_kw_wconinje_type * kw , const char * well_name);
bool                     sched_kw_wconinje_well_open( const sched_kw_wconinje_type * kw, const char * well_name);
char **                  sched_kw_wconinje_alloc_wells_copy( const sched_kw_wconinje_type * , int * );

void                     sched_kw_wconinje_set_surface_flow( const sched_kw_wconinje_type * kw , const char * well, double surface_flow);
void                     sched_kw_wconinje_scale_surface_flow( const sched_kw_wconinje_type * kw , const char * well, double factor);
double                   sched_kw_wconinje_get_surface_flow( const sched_kw_wconinje_type * kw , const char * well);
bool                     sched_kw_wconinje_has_well( const sched_kw_wconinje_type * , const char * );
sched_kw_wconinje_type * sched_kw_wconinje_safe_cast( void * arg );
void                     sched_kw_wconinje_shift_surface_flow( const sched_kw_wconinje_type * kw , const char * well_name , double delta_surface_flow);
bool                     sched_kw_wconinje_buffer_fwrite( const sched_kw_wconinje_type * kw , const char * well_name , buffer_type * buffer);


void                     sched_kw_wconinje_close_state(wconinje_state_type * state , int report_step );
void                     sched_kw_wconinje_update_state( const sched_kw_wconinje_type * kw , wconinje_state_type * state , const char * well_name , int report_step );
void                     wconinje_state_free__( void * arg );
wconinje_state_type    * wconinje_state_alloc( const char * well_name , const time_t_vector_type * time);
void                     wconinje_state_free( wconinje_state_type * wconinje );
double                   wconinje_state_iget_WGIRH( const void * __state , int report_step );
double                   wconinje_state_iget_WWIRH( const void * __state , int report_step );
void                     sched_kw_wconinje_init_well_list( const sched_kw_wconinje_type * kw , stringlist_type * well_list);

/*******************************************************************/



KW_HEADER(wconinje)



#ifdef __cplusplus
}
#endif
#endif
