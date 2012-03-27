/*
   Copyright (C) 2011  Statoil ASA, Norway. 
    
   The file 'ecl_file_kw.c' is part of ERT - Ensemble based Reservoir Tool. 
    
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


#include <stdlib.h>
#include <stdio.h>

#include <util.h>

#include <ecl_util.h>
#include <ecl_kw.h>
#include <ecl_file_kw.h>
#include <fortio.h>

/*
  This file implements the datatype ecl_file_kw which is used to hold
  header-information about an ecl_kw instance on file. When a
  ecl_file_kw instance is created it is initialized with the header
  information (name, size, type) for an ecl_kw instance and the offset
  in a file actually containing the keyword.

  If and when the keyword is actually queried for at a later stage the
  ecl_file_kw_get_kw() method will seek to the keyword position in an
  open fortio instance and call ecl_kw_fread_alloc() to instantiate
  the keyword itself. 

  The ecl_file_kw datatype is mainly used by the ecl_file datatype;
  whose index tables consists of ecl_file_kw instances.
*/


#define ECL_FILE_KW_TYPE_ID 646107

struct ecl_file_kw_struct {
  UTIL_TYPE_ID_DECLARATION
  long             file_offset;
  ecl_type_enum    ecl_type;
  int              kw_size;
  char           * header;
  ecl_kw_type    * kw;
};

static UTIL_SAFE_CAST_FUNCTION( ecl_file_kw , ECL_FILE_KW_TYPE_ID )




static ecl_file_kw_type * ecl_file_kw_alloc__( const char * header , ecl_type_enum ecl_type , int size , long offset) {
  ecl_file_kw_type * file_kw = util_malloc( sizeof * file_kw , __func__);
  UTIL_TYPE_ID_INIT( file_kw , ECL_FILE_KW_TYPE_ID );
  
  file_kw->header = util_alloc_string_copy( header );
  file_kw->kw_size = size;
  file_kw->ecl_type = ecl_type;
  file_kw->file_offset = offset;
  file_kw->kw = NULL;

  return file_kw;
}

/**
   Create a new ecl_file_kw instance based on header information from
   the input keyword. Typically only the header has been loaded from
   the keyword.  

   Observe that it is the users responsability that the @offset
   argument in ecl_file_kw_alloc() comes from the same fortio instance
   as used when calling ecl_file_kw_get_kw() to actually instatiate
   the ecl_kw. This is automatically assured when using ecl_file to
   access the ecl_file_kw instances.  
*/

ecl_file_kw_type * ecl_file_kw_alloc( const ecl_kw_type * ecl_kw , long offset ) {
  return ecl_file_kw_alloc__( ecl_kw_get_header( ecl_kw ) , ecl_kw_get_type( ecl_kw ) , ecl_kw_get_size( ecl_kw ) , offset );
}


/** 
    Does NOT copy the kw pointer which must be reloaded.
*/
ecl_file_kw_type * ecl_file_kw_alloc_copy( const ecl_file_kw_type * src ) {
  return ecl_file_kw_alloc__( src->header , src->ecl_type , src->kw_size , src->file_offset );
}


static void ecl_kw_drop_kw( ecl_file_kw_type * file_kw ) {
  if (file_kw->kw != NULL) {
    ecl_kw_free( file_kw->kw );
    file_kw->kw = NULL;
  }
}


void ecl_file_kw_free( ecl_file_kw_type * file_kw ) {
  ecl_kw_drop_kw( file_kw );
  free( file_kw->header );
  free( file_kw );
}


void ecl_file_kw_free__( void * arg ) {
  ecl_file_kw_type * file_kw = ecl_file_kw_safe_cast( arg );
  ecl_file_kw_free( file_kw );
}



static void ecl_file_kw_assert_kw( const ecl_file_kw_type * file_kw ) {
  if (file_kw->ecl_type != ecl_kw_get_type( file_kw->kw )) 
    util_abort("%s: type mismatch between header and file.\n",__func__);

  if (file_kw->kw_size != ecl_kw_get_size( file_kw->kw )) 
    util_abort("%s: size mismatch between header and file.\n",__func__);

  if (strcmp( file_kw->header , ecl_kw_get_header( file_kw->kw )) != 0 )
    util_abort("%s: name mismatch between header and file.\n",__func__);
}


static void ecl_file_kw_load_kw( ecl_file_kw_type * file_kw , fortio_type * fortio ) {
  if (fortio == NULL)
    util_abort("%s: trying to load a keyword after the backing file has been detached.\n",__func__);
  
  if (file_kw->kw != NULL)
    ecl_kw_free( file_kw->kw );
  {
    fortio_fseek( fortio , file_kw->file_offset , SEEK_SET );
    file_kw->kw = ecl_kw_fread_alloc( fortio );
    ecl_file_kw_assert_kw( file_kw );
  }
}


/*
  Will return the ecl_kw instance of this file_kw; if it is not
  currently loaded the method will instantiate the ecl_kw instance
  from the @fortio input handle. 

  After loading the keyword it will be kept in memory, so a possible
  subsequent lookup will be served from memory.  
*/


ecl_kw_type * ecl_file_kw_get_kw( ecl_file_kw_type * file_kw , fortio_type * fortio) {
  if (file_kw->kw == NULL)
    ecl_file_kw_load_kw( file_kw , fortio );
  return file_kw->kw;
}


bool ecl_file_kw_ptr_eq( const ecl_file_kw_type * file_kw , const ecl_kw_type * ecl_kw) {
  if (file_kw->kw == ecl_kw)
    return true;
  else
    return false;
}



void ecl_file_kw_replace_kw( ecl_file_kw_type * file_kw , fortio_type * target , ecl_kw_type * new_kw ) {
  if ((file_kw->ecl_type == ecl_kw_get_type( new_kw )) && 
      (file_kw->kw_size == ecl_kw_get_size( new_kw ))) {
    
    if (file_kw->kw != NULL)
      ecl_kw_free( file_kw->kw );

    file_kw->kw = new_kw;
    fortio_fseek( target , file_kw->file_offset , SEEK_SET );
    ecl_kw_fwrite( file_kw->kw , target );
    
  } else
    util_abort("%s: sorry size/type mismatch between in-file keyword and new keyword \n",__func__);
}



const char * ecl_file_kw_get_header( const ecl_file_kw_type * file_kw ) {
  return file_kw->header;
}

int ecl_file_kw_get_size( const ecl_file_kw_type * file_kw ) {
  return file_kw->kw_size;
}

ecl_type_enum ecl_file_kw_get_type( const ecl_file_kw_type * file_kw) {
  return file_kw->ecl_type;
}


void ecl_file_kw_fwrite( ecl_file_kw_type * file_kw , fortio_type * src , fortio_type * target) {
  ecl_kw_type * ecl_kw = ecl_file_kw_get_kw( file_kw , src);
  ecl_kw_fwrite( ecl_kw , target );
}


void ecl_file_kw_fskip_data( const ecl_file_kw_type * file_kw , fortio_type * fortio) {
  ecl_kw_fskip_data__( file_kw->ecl_type , file_kw->kw_size , fortio );
}


