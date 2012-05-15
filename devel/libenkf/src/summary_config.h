/*
   Copyright (C) 2011  Statoil ASA, Norway. 
    
   The file 'summary_config.h' is part of ERT - Ensemble based Reservoir Tool. 
    
   ERT is free software: you can redistribute it and/or modify 
   it under the terms of the GNU General Public License as published by 
   the Free Software Foundation, either version 3 of the License, or 
   (at your option) any later version. 
    
   ERT is distributed in the hope that it will be useful, but WITHOUT ANY 
   WARRANTY; without even the implied warranty of MERCHANTABILITY or 
   FITNESS FOR A PARTICULAR PURPOSE.   
    
   See the GNU General Public License at <http://www.gnu.org/licenses/gpl.html> 
   for more details. 
*/

#ifndef __SUMMARY_CONFIG_H__
#define __SUMMARY_CONFIG_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <stdlib.h>

#include <util.h>

#include <ecl_sum.h>
#include <ecl_smspec.h>

#include <enkf_macros.h>


  typedef struct summary_config_struct summary_config_type;
  typedef struct summary_struct        summary_type;
  
  bool                   summary_config_get_vector_storage( const summary_config_type * config);
  ecl_smspec_var_type    summary_config_get_var_type(summary_config_type * , const ecl_sum_type * ecl_sum);
  const           char * summary_config_get_var(const summary_config_type * );
  void                   summary_config_set_obs_config_file(summary_config_type * , const char * );
  const char           * summary_config_get_config_txt_file_ref(const summary_config_type * );
  summary_config_type  * summary_config_alloc(const char * ,  bool vector_storage);
  void                   summary_config_free(summary_config_type * );
  int                    summary_config_get_active_mask(const summary_config_type *);
  int                    summary_config_get_var_index(const summary_config_type * , const char * );
  const char          ** summary_config_get_var_list_ref(const summary_config_type *);
  void                   summary_config_add_var(summary_config_type *  , const char * );
  bool                   summary_config_has_var(const summary_config_type * , const char * );
  void                   summary_config_summarize(const summary_config_type * );
  void                   summary_config_add_obs_key(summary_config_type * , const char * );
  int                    summary_config_get_byte_size(const summary_config_type * );
  
  UTIL_SAFE_CAST_HEADER(summary_config);
  UTIL_SAFE_CAST_HEADER_CONST(summary_config);
  GET_DATA_SIZE_HEADER(summary);
  VOID_GET_DATA_SIZE_HEADER(summary);
  VOID_CONFIG_FREE_HEADER(summary);
  
  

#ifdef __cplusplus
}
#endif
#endif
