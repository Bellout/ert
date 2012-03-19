/*
   Copyright (C) 2012  Statoil ASA, Norway. 
    
   The file 'enkf_plot_data.h' is part of ERT - Ensemble based Reservoir Tool. 
    
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


#ifndef __ENKF_PLOT_DATA_H__
#define __ENKF_PLOT_DATA_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <time.h>
#include <member_config.h>


  
  typedef struct enkf_plot_data_struct enkf_plot_data_type;
  
  enkf_plot_data_type * enkf_plot_data_alloc( time_t start_time);
  void                 enkf_plot_data_free( enkf_plot_data_type * plot_data );
  void                 enkf_plot_data_append_member( enkf_plot_data_type * plot_data , member_config_type * member_config );
  void                 enkf_plot_data_load( enkf_plot_data_type * plot_data , enkf_config_node_type * config_node , enkf_fs_type * fs , const char * user_key , state_enum state , int step1 , int step2);


#ifdef __cplusplus
}
#endif
#endif
