/*
   Copyright (C) 2011  Statoil ASA, Norway. 
    
   The file 'plain_driver.h' is part of ERT - Ensemble based Reservoir Tool. 
    
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

#ifndef __PLAIN_DRIVER_H__
#define __PLAIN_DRIVER_H__

#include <stdio.h>
#include <fs_types.h>
#include <stdbool.h>

typedef struct plain_driver_struct plain_driver_type;

void                 plain_driver_fwrite_mount_info(FILE * stream , fs_driver_enum driver_type , const char * fmt);
plain_driver_type  * plain_driver_fread_alloc(const char * root_path , FILE * stream);

#endif
