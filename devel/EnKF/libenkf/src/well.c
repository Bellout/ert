#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <config.h>
#include <enkf_util.h>
#include <util.h>
#include <well.h>
#include <well_config.h>
#include <ecl_sum.h>
#include <enkf_types.h>


#define  DEBUG
#define  TARGET_TYPE WELL
#include "enkf_debug.h"


/*****************************************************************/

struct well_struct {
  DEBUG_DECLARE
  const well_config_type * config;
  double *data;
};



void well_clear(well_type * well) {
  const int size = well_config_get_data_size(well->config);   
  int k;
  for (k = 0; k < size; k++)
    well->data[k] = 0.0;
}


void well_realloc_data(well_type *well) {
  well->data = enkf_util_calloc(well_config_get_data_size(well->config) , sizeof *well->data , __func__);
}


void well_free_data(well_type *well) {
  free(well->data);
  well->data = NULL;
}


well_type * well_alloc(const well_config_type * well_config) {
  well_type * well  = malloc(sizeof *well);
  well->config = well_config;
  well->data = NULL;
  well_realloc_data(well);
  DEBUG_ASSIGN(well)
  return well;
}



char * well_alloc_ensfile(const well_type * well , const char * path) {
  return util_alloc_full_path(path , well_config_get_ensfile_ref(well->config));
}


well_type * well_copyc(const well_type *well) {
  const int size = well_config_get_data_size(well->config);   
  well_type * new = well_alloc(well->config);
  
  memcpy(new->data , well->data , size * sizeof *well->data);
  return new;
}


void well_fread(well_type * well , const char *file) {
  FILE * stream   = enkf_util_fopen_r(file , __func__);
  int  size;
  fread(&size , sizeof  size , 1 , stream);
  enkf_util_fread(well->data , sizeof *well->data , size , stream , __func__);
  fclose(stream);
}


void well_fwrite(const well_type * well , const char *file) {
  const  well_config_type * config = well->config;
  const int data_size = well_config_get_data_size(config);
  FILE * stream       = enkf_util_fopen_w(file , __func__);
  
  fwrite(&data_size            , sizeof  data_size     , 1 , stream);
  enkf_util_fwrite(well->data  , sizeof *well->data    ,data_size , stream , __func__);
  
  fclose(stream);
}



void well_ens_read(well_type * well , const char *path) {
  char * ensfile = util_alloc_full_path(path , well_config_get_ensfile_ref(well->config));
  well_fread(well , ensfile);
  free(ensfile);
}


void well_ens_write(const well_type * well , const char * path) {
  DEBUG_ASSERT(well)
  {
    char * ensfile = util_alloc_full_path(path , well_config_get_ensfile_ref(well->config));
    well_fwrite(well , ensfile);
    free(ensfile);
  }
}


char * well_swapout(well_type * well , const char * path) {
  char * ensfile = util_alloc_full_path(path , well_config_get_ensfile_ref(well->config));
  well_fwrite(well , ensfile);
  well_free_data(well);
  return ensfile;
}


void well_swapin(well_type * well , const char *file) {
  well_realloc_data(well);
  well_fread(well  , file);
}



void well_free(well_type *well) {
  well_free_data(well);
  free(well);
}



int well_serialize(const well_type *well , int internal_offset , size_t serial_data_size , double *serial_data , size_t stride , size_t offset, bool * complete) {
  DEBUG_ASSERT(well);
  {
    const well_config_type * config = well->config;
    const int data_size          = well_config_get_data_size(config);
    const int max_serial_index   = (serial_data_size - offset) / stride;
    const int max_internal_index = util_int_min(data_size , max_serial_index);
    int internal_index;
    
    for (internal_index = internal_offset; internal_index < max_internal_index; internal_index++)
      serial_data[offset + internal_index*stride] = well->data[internal_index];
    
    if (max_internal_index < data_size)
      *complete = false;
    else
    *complete = true;
    
    return (max_internal_index - internal_offset);
  }
}


int well_deserialize(well_type *well , int internal_offset , size_t serial_size , const double * serial_data , size_t stride , size_t offset) {
  DEBUG_ASSERT(well);
  {
    int internal_index;
    
    for (internal_index = internal_offset; internal_index < (serial_size + internal_offset); internal_index++)
      well->data[internal_index] = serial_data[offset + internal_index*stride];
    
    return serial_size + internal_offset;
  }
}


double well_get(const well_type * well, const char * var) {
  const well_config_type *config       = well->config;
  int index                            = well_config_get_var_index(config , var);
  if (index < 0) {
    fprintf(stderr,"%s: well:%s does not have variable:%s - aborting \n",__func__ , well_config_get_well_name_ref(config) , var);
    abort();
  }
  return well->data[index];
}


void well_load_summary_data(well_type * well , int report_step , const ecl_sum_type * ecl_sum) {
  DEBUG_ASSERT(well)
  {
    const well_config_type *config       = well->config;
    const char ** var_list               = well_config_get_var_list_ref(config);
    const char *  well_name              = well_config_get_well_name_ref(config);
    int ivar;
    
    for (ivar = 0; ivar < well_config_get_data_size(config); ivar++) 
  	well->data[ivar] = ecl_sum_iget1(ecl_sum , report_step , well_name , var_list[ivar] , NULL);
  }
}


MATH_OPS(well)
VOID_ALLOC(well)
VOID_FREE(well)
VOID_FREE_DATA(well)
VOID_REALLOC_DATA(well)
VOID_ENS_WRITE (well)
VOID_ENS_READ  (well)
VOID_COPYC     (well)
VOID_SWAPIN(well)
VOID_SWAPOUT(well)
VOID_SERIALIZE(well)
VOID_DESERIALIZE(well)
/******************************************************************/
/* Anonumously generated functions used by the enkf_node object   */
/******************************************************************/

VOID_FUNC      (well_clear        , well_type)


