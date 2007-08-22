#ifndef __ENKF_STATE_H__
#define __ENKF_STATE_H__

#include <fortio.h>
#include <stdbool.h>
#include <enkf_config.h>
#include <enkf_types.h>
#include <enkf_node.h>
#include <enkf_util.h>
#include <ecl_block.h>


typedef struct enkf_state_struct enkf_state_type;

void              enkf_state_swapout(enkf_state_type * , int );
void              enkf_state_swapin(enkf_state_type * , int );
enkf_state_type * enkf_state_copyc(const enkf_state_type * );
void              enkf_state_iset_eclpath(enkf_state_type * , int , const char *);
void              enkf_state_add_node(enkf_state_type * , const char * );
enkf_state_type * enkf_state_alloc(const enkf_config_type * , const char *, bool );
enkf_node_type  * enkf_state_get_node(const enkf_state_type * , const char * );
void              enkf_state_del_node(enkf_state_type * , const char * );
void              enkf_state_load_ecl_summary(enkf_state_type * , bool , int );
void            * enkf_state_load_ecl_summary_void(void * );
void            * enkf_state_load_ecl_restart_void(void * );
void            * enkf_state_load_ecl_void(void * );
void              enkf_state_load_ecl(enkf_state_type * , bool , int );
int               enkf_state_get_serial_size(const enkf_state_type * );

void              enkf_state_iset_enspath(enkf_state_type * , int , const char *);
const      char * enkf_state_get_enspath_ref(const enkf_state_type * );
void              enkf_state_load_ecl_restart(enkf_state_type * , bool , int );
void              enkf_state_sample(enkf_state_type * , int);
void              enkf_state_ens_write(const enkf_state_type * , int);
void              enkf_state_ens_read(       enkf_state_type * , const char * , int);
void 		  enkf_state_ecl_write(const enkf_state_type * , int , int);
void              enkf_state_ecl_read(enkf_state_type * , const ecl_block_type *);
void              enkf_state_free(enkf_state_type * );
void              enkf_state_apply(enkf_state_type * , enkf_node_ftype1 * , int );
void              enkf_state_serialize(enkf_state_type *);
void              enkf_state_set_serial_data(enkf_state_type * , double * );
#endif
