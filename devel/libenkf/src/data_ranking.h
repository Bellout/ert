/*
   Copyright (C) 2012  Statoil ASA, Norway. 
    
   The file 'data_ranking.h' is part of ERT - Ensemble based Reservoir Tool. 
    
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

#ifndef __DATA_RANKING_H__
#define __DATA_RANKING_H__

#ifdef __cplusplus
extern "C" {
#endif

  #include <type_macros.h>

#include <enkf_fs.h>

  typedef struct data_ranking_struct data_ranking_type;

  UTIL_IS_INSTANCE_HEADER( data_ranking );
  UTIL_SAFE_CAST_HEADER(data_ranking);

  data_ranking_type * data_ranking_alloc( int ens_size , const char * user_key , const char * key_index , enkf_fs_type * fs , enkf_config_node_type * config_node , int step , state_enum state);
  void                data_ranking_free__( void * arg );


#ifdef __cplusplus
}
#endif
#endif





