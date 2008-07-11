#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <hash.h>
#include <list.h>
#include <list_node.h>
#include <util.h>

#include <rms_type.h>
#include <rms_util.h>
#include <rms_tag.h>
#include <rms_file.h>
#include <rms_tagkey.h>
#include <ecl_kw.h>

/*****************************************************************/
static const char * rms_ascii_header      = "roff-asc";
static const char * rms_binary_header     = "roff-bin";

static const char * rms_comment1          = "ROFF file";
static const char * rms_comment2          = "Creator: RMS - Reservoir Modelling System, version 8.1";
/*
  static const char * rms_parameter_tagname = "parameter";
*/





struct rms_file_struct {
  char       * filename;
  bool         endian_convert;
  bool         fmt_file;
  hash_type  * type_map;
  list_type  * tag_list;
  FILE       * stream;
};



/*****************************************************************/
/* Pure roff routines */




static bool rms_fmt_file(const rms_file_type *rms_file) {
  bool fmt_file;
  char filetype[9];
  rms_util_fread_string( filetype , 9 , rms_file->stream);

  if (strncmp(filetype , rms_binary_header , 8) == 0)
    fmt_file = false;
  else if (strncmp(filetype , rms_ascii_header , 8) == 0)
    fmt_file = true;
  else {
    fprintf(stderr,"%s: header : %8s not recognized in file: %s - aborting \n",__func__ , filetype , rms_file->filename);
    abort();
  }
  return fmt_file;
}


    
static void rms_file_add_tag(rms_file_type *rms_file , const rms_tag_type *tag) {
  list_append_ref(rms_file->tag_list , tag);
}


void rms_file_add_dimensions(rms_file_type * rms_file , int nX , int nY , int nZ , bool save) {
  if (rms_file_get_tag_ref(rms_file , "dimensions" , NULL , NULL , false) != NULL) {
    fprintf(stderr,"%s: dimensions tag already persent in rms_file object - aborting \n",__func__);
    abort();
  }
  {
    rms_tag_type * dim_tag = rms_tag_alloc_dimensions(nX , nY , nZ);
    rms_file_add_tag(rms_file , dim_tag);
    if (save) 
      rms_tag_fwrite(dim_tag , rms_file->stream);
  }
}


rms_tag_type * rms_file_get_tag_ref(const rms_file_type *rms_file , const char *tagname, const char *keyname, const char *keyvalue, bool abort_on_error) {
  bool cont;
  rms_tag_type *return_tag = NULL;
  list_node_type *tag_node = list_get_head(rms_file->tag_list);
  if (tag_node != NULL)
    cont = true;
  else
    cont = false;

  while (cont) {
    rms_tag_type *tag = list_node_value_ptr(tag_node);
    if (rms_tag_name_eq(tag , tagname , keyname , keyvalue)) {
      return_tag = tag;
      cont = false;
    } else 
      tag_node = list_node_get_next(tag_node);
    if (tag_node == NULL)
      cont = false;
  }
  
  
  if (return_tag == NULL && abort_on_error) {
    if (keyname != NULL && keyvalue != NULL) 
      fprintf(stderr,"%s: failed to find tag:%s with key:%s=%s in file:%s - aborting \n",__func__ , tagname , keyname , keyvalue , rms_file->filename);
    else
      fprintf(stderr,"%s: failed to find tag:%s in file:%s - aborting \n",__func__ , tagname , rms_file->filename);
  }
  return return_tag;
}


list_node_type * rms_file_iterate_tag_node(const rms_file_type * rms_file , list_node_type * tag_node) {
  if (tag_node == NULL)
    return list_get_head(rms_file->tag_list);
  else 
    return list_node_get_next(tag_node);
}




/** 
    This function allocates and rms_file_type * handle, but it does
    not load the file content. 
*/


rms_file_type * rms_file_alloc(const char *filename, bool fmt_file) {
  rms_file_type *rms_file   = malloc(sizeof *rms_file);
  rms_file->endian_convert  = false;
  rms_file->type_map  	    = hash_alloc();
  rms_file->tag_list  	    = list_alloc();
  
  hash_insert_hash_owned_ref(rms_file->type_map , "byte"   , rms_type_alloc(rms_byte_type ,    1) ,  rms_type_free);
  hash_insert_hash_owned_ref(rms_file->type_map , "bool"   , rms_type_alloc(rms_bool_type,     1) ,  rms_type_free);
  hash_insert_hash_owned_ref(rms_file->type_map , "int"    , rms_type_alloc(rms_int_type ,     4) ,  rms_type_free);
  hash_insert_hash_owned_ref(rms_file->type_map , "float"  , rms_type_alloc(rms_float_type  ,  4) ,  rms_type_free);
  hash_insert_hash_owned_ref(rms_file->type_map , "double" , rms_type_alloc(rms_double_type ,  8) ,  rms_type_free);

  hash_insert_hash_owned_ref(rms_file->type_map , "char"   , rms_type_alloc(rms_char_type   , -1) ,  rms_type_free);   /* Char are a f*** mix of vector and scalar */

  rms_file->filename = NULL;
  rms_file->stream   = NULL;
  rms_file_set_filename(rms_file , filename , fmt_file);
  return rms_file;
}






void rms_file_set_filename(rms_file_type * rms_file , const char *filename , bool fmt_file) {
  rms_file->filename = util_realloc_string_copy(rms_file->filename , filename);
  rms_file->fmt_file   = fmt_file;
}


static void rms_file_delete_tag(rms_file_type * rms_file , list_node_type *tag_node) {
  rms_tag_free( list_node_value_ptr(tag_node) );
  list_del_node(rms_file->tag_list , tag_node);
}


void rms_file_free_data(rms_file_type * rms_file) {
  list_node_type    *tag_node , *next_tag_node;
  list_type         *tag_list = rms_file->tag_list;
    
  tag_node = list_get_head(tag_list);
  while (tag_node) {
    next_tag_node = list_node_get_next(tag_node);
    rms_file_delete_tag(rms_file , tag_node);
    tag_node = next_tag_node;
  }
}



void rms_file_free(rms_file_type * rms_file) {
  rms_file_free_data(rms_file);
  list_free(rms_file->tag_list);
  hash_free(rms_file->type_map);
  free(rms_file->filename);
  free(rms_file);
}


static int rms_file_get_dim(const rms_tag_type *tag , const char *dim_name) {
  rms_tagkey_type *key = rms_tag_get_key(tag , dim_name);
  if (key == NULL) {
    fprintf(stderr,"%s: failed to find tagkey:%s aborting \n" , __func__ , dim_name);
    abort();
  }
  return * (int *) rms_tagkey_get_data_ref(key);
}



void rms_file_assert_dimensions(const rms_file_type *rms_file , int nx , int ny , int nz) {
  bool OK = true;  
  rms_tag_type    *tag    = rms_file_get_tag_ref(rms_file , "dimensions" , NULL , NULL , true);
  OK =       (nx == rms_file_get_dim(tag , "nX"));
  OK = OK && (ny == rms_file_get_dim(tag , "nY"));
  OK = OK && (nz == rms_file_get_dim(tag , "nZ"));

  if (!OK) {
    fprintf(stderr,"%s: dimensions on file: %s (%d, %d, %d) did not match with input dimensions (%d,%d,%d) - aborting \n",__func__ , rms_file->filename,
	    rms_file_get_dim(tag , "nX"), rms_file_get_dim(tag , "nY"), rms_file_get_dim(tag , "nZ"),
	    nx , ny , nz);
    abort();
  }
}

rms_tag_type * rms_file_get_dim_tag_ref(const rms_file_type * rms_file) {
  return rms_file_get_tag_ref(rms_file , "dimensions" , NULL , NULL , true);
}


void rms_file_get_dims(const rms_file_type * rms_file , int * dims) {
  rms_tag_type *tag = rms_file_get_dim_tag_ref(rms_file);
  dims[0] = rms_file_get_dim(tag , "nX");
  dims[1] = rms_file_get_dim(tag , "nY");
  dims[2] = rms_file_get_dim(tag , "nZ");
}


FILE * rms_file_get_FILE(const rms_file_type * rms_file) { return rms_file->stream; }


static void rms_file_init_fread(rms_file_type * rms_file) {

  rms_file->fmt_file = rms_fmt_file( rms_file );
  if (rms_file->fmt_file) {
    fprintf(stderr,"%s only binary files implemented - aborting \n",__func__);
    abort();
  }
  /* Skipping two comment lines ... */
  rms_util_fskip_string(rms_file->stream);
  rms_util_fskip_string(rms_file->stream);  
  {
    bool eof_tag;
    rms_tag_type    * filedata_tag = rms_tag_fread_alloc(rms_file->stream , rms_file->type_map , rms_file->endian_convert , &eof_tag);
    rms_tagkey_type * byteswap_key = rms_tag_get_key(filedata_tag , "byteswaptest");
    if (byteswap_key == NULL) {
      fprintf(stderr,"%s: failed to find filedata/byteswaptest - aborting \n", __func__);
      abort();
    }
    int byteswap_value             = *( int *) rms_tagkey_get_data_ref(byteswap_key);
    if (byteswap_value == 1)
      rms_file->endian_convert = false;
    else
      rms_file->endian_convert = true;
    rms_tag_free(filedata_tag);
  }
}



rms_tag_type * rms_file_fread_alloc_tag(rms_file_type * rms_file , const char *tagname , const char * keyname , const char *keyvalue ) {
  rms_tag_type * tag = NULL;
  rms_file_fopen_r(rms_file);
  {
  
    bool cont          = true;
    bool tag_found     = false;
    long int start_pos = ftell(rms_file->stream);
    fseek(rms_file->stream , 0 , SEEK_SET);
    rms_file_init_fread(rms_file);
    while (cont) {
      bool eof_tag;
      rms_tag_type * tmp_tag = rms_tag_fread_alloc(rms_file->stream , rms_file->type_map , rms_file->endian_convert , &eof_tag);
      if (rms_tag_name_eq(tmp_tag , tagname , keyname , keyvalue)) {
	tag_found = true;
	tag = tmp_tag;
      } else 
	rms_tag_free(tmp_tag);
      if (tag_found || eof_tag)
	cont = false;
    }
    if (tag == NULL) {
      fseek(rms_file->stream , start_pos , SEEK_SET);
      fprintf(stderr,"%s: could not find tag: \"%s\" (with %s=%s) in file:%s - aborting.\n",__func__ , tagname , keyname , keyvalue , rms_file->filename);
      abort();
    }
  }
  rms_file_fclose(rms_file);
  return tag;
}



FILE * rms_file_fopen_r(rms_file_type *rms_file) {
  rms_file->stream = util_fopen(rms_file->filename , "r");
  return rms_file->stream;
}


FILE * rms_file_fopen_w(rms_file_type *rms_file) {
  rms_file->stream = util_fopen(rms_file->filename , "w");
  return rms_file->stream;
}

void rms_file_fclose(rms_file_type * rms_file) {
  fclose(rms_file->stream);
  rms_file->stream = NULL;
}


rms_tagkey_type * rms_file_fread_alloc_data_tagkey(rms_file_type * rms_file , const char *tagname , const char * keyname , const char *keyvalue) {
  rms_tag_type * tag = rms_file_fread_alloc_tag(rms_file , tagname , keyname , keyvalue);
  if (tag != NULL) {
    rms_tagkey_type *tagkey = rms_tagkey_copyc( rms_tag_get_key(tag , "data") );
    rms_tag_free(tag);
    return tagkey;
  } else
    return NULL;
}



void rms_file_fread(rms_file_type *rms_file) {
  rms_file_fopen_r(rms_file);
  rms_file_init_fread(rms_file);
  
  /* The main read loop */
  {
    bool eof_tag = false;
    while (!eof_tag) {
      rms_tag_type * tag = rms_tag_fread_alloc(rms_file->stream ,  rms_file->type_map , rms_file->endian_convert , &eof_tag );
      if (!eof_tag)
	rms_file_add_tag(rms_file , tag);
      else
	rms_tag_free(tag);
      
    }
  }
  rms_file_fclose(rms_file);
}



/*static */
void rms_file_init_fwrite(const rms_file_type * rms_file , const char * filetype) {
  if (!rms_file->fmt_file)
    rms_util_fwrite_string(rms_binary_header , rms_file->stream);
  else {
    fprintf(stderr,"%s: Sorry only binary writes implemented ... \n",__func__);
    rms_util_fwrite_string(rms_ascii_header , rms_file->stream);
  }
  
  rms_util_fwrite_comment(rms_comment1 , rms_file->stream);
  rms_util_fwrite_comment(rms_comment2 , rms_file->stream);
  rms_tag_fwrite_filedata(filetype , rms_file->stream);
}



void rms_file_complete_fwrite(const rms_file_type * rms_file) {
  rms_tag_fwrite_eof(rms_file->stream);
}



void rms_file_fwrite(rms_file_type * rms_file, const char * filetype) {
  rms_file_fopen_w(rms_file);
  rms_file_init_fwrite(rms_file , filetype );

  {
    list_node_type * tag_node = list_get_head(rms_file->tag_list);
    while (tag_node != NULL) {
      const rms_tag_type *tag = list_node_value_ptr(tag_node);
      rms_tag_fwrite(tag , rms_file->stream);
      tag_node = list_node_get_next(tag_node);
    }
  }
  rms_file_complete_fwrite(rms_file );
  rms_file_fclose(rms_file);
}


void rms_file_printf(const rms_file_type *rms_file , FILE *stream) {
  fprintf(stream , "<%s>\n",rms_file->filename);
  {
    list_node_type * tag_node = list_get_head(rms_file->tag_list);
    while (tag_node != NULL) {
      const rms_tag_type *tag = list_node_value_ptr(tag_node);
      rms_tag_printf(tag , stream);
      tag_node = list_node_get_next(tag_node);
    }
  }
  fprintf(stream , "</%s>\n",rms_file->filename);
}



/*
  Hardcoded assumption that the parameter type is float - otherwise this
  will break hard.
*/

void rms_file_2eclipse(const char * rms_file , const char * ecl_path, bool ecl_fmt_file , bool ecl_endian_convert , int ecl_file_nr) {
  char * rms_base_file;
  int dims[3] , size;
  rms_file_type *file = rms_file_alloc(rms_file , false);
  rms_file_fread(file);
  rms_file_get_dims(file , dims);
  size = dims[0] * dims[1] * dims[2] ;
  
  util_alloc_file_components(rms_file , NULL , &rms_base_file , NULL);
  {
    list_node_type * tag_node = NULL;
    float * ecl_data = malloc(size * sizeof * ecl_data);
    do {
      tag_node = rms_file_iterate_tag_node(file , tag_node);
      
      if (tag_node != NULL) {
	rms_tag_type * rms_tag = list_node_value_ptr(tag_node);
	if (rms_tag_name_eq(rms_tag , "parameter" , NULL , NULL)) {
	  rms_tagkey_type * rms_tagkey = rms_tag_get_datakey(rms_tag);
	  const float * data = rms_tagkey_get_data_ref(rms_tagkey);
	  rms_util_set_fortran_data(ecl_data , data , sizeof * ecl_data , dims[0] , dims[1] , dims[2]);

	  {
	    float rms_undef = -999;
	    float ecl_undef = 0;
	    rms_util_translate_undef(ecl_data , size , rms_tagkey_get_sizeof_ctype(rms_tagkey) , &rms_undef , &ecl_undef);
	  }
	  
	  {
	    const char * tagname  = rms_tag_get_namekey_name(rms_tag);
	    char       * ecl_base = malloc(4 + strlen(tagname) + 2);
	    char       * ecl_file;
	    
	    sprintf(ecl_base , "%s_%04d" , tagname , ecl_file_nr);
	    ecl_file = util_alloc_full_path(ecl_path , ecl_base);
	    if (util_same_file(ecl_file , rms_file)) {
	      fprintf(stderr,"%s: attempt to overwrite %s -> %s - aborting \n",__func__ , rms_file , ecl_file);
	      abort();
	    }
	    
	    ecl_kw_fwrite_param(ecl_file , ecl_fmt_file , ecl_endian_convert , tagname , ecl_float_type , size , ecl_data);
	    free(ecl_base);
	    free(ecl_file);
	    
	  }
	}
      }
    } while (tag_node != NULL);
    free(ecl_data);
  }
  free(rms_base_file);
}



bool rms_file_is_roff(FILE * stream) {
  const int len        	     = strlen(rms_comment1);
  char *header         	     = malloc(strlen(rms_comment1) + 1);
  const long int current_pos = ftell(stream);
  bool roff_file             = false;

  fseek(stream , 1 + 1 + 8 , SEEK_CUR); /* Skipping #roff-bin#0#  WILL Fail with formatted files */
  rms_util_fread_string(header , len+1 , stream);
  if (strncmp(rms_comment1 , header , len) == 0)
    roff_file = true;
  
  fseek(stream , current_pos , SEEK_SET);
  free(header);
  return roff_file;
}


/*****************************************************************/
/* Old hack version: */
  

void old_rms_roff_load(const char *filename , const char *param_name , float *param) {
  const int offset = 327 + strlen(param_name);
  int n_read;
  int size;
  FILE *stream     = fopen(filename , "r");
  
  fseek(stream , offset , SEEK_SET);
  fread(&size  , 1 , sizeof size , stream);
  n_read = fread(param , sizeof *param , size , stream);
  
  fclose(stream);
  if (n_read != size) {
    fprintf(stderr,"%s: wanted:%d elements - only read:%d - aborting \n",__func__, size , n_read);
    abort();
  }
}
