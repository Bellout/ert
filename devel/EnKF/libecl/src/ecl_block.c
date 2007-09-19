#include <stdio.h>
#include <string.h>
#include <math.h>
#include <fortio.h>
#include <ecl_kw.h>
#include <ecl_block.h>
#include <errno.h>
#include <hash.h>
#include <list.h>
#include <util.h>
#include <time.h>


struct ecl_block_struct {
  bool 	        fmt_file;
  bool 	        endian_convert;

  /*
    This code is programmed in terms of "report steps", in ECLIPSE speak,
    it has no understanding of the socalled ministeps, and will probably break
    badly if exposed to them.
  */
  
  int           report_nr;
  /*int           block_nr;*/
  int           size;
  hash_type    *kw_hash;
  list_type    *kw_list;
  time_t        sim_time;
};



/*
  hash_node -> list_node -> ecl_kw
*/


bool ecl_block_add_kw(ecl_block_type *ecl_block , const ecl_kw_type *ecl_kw, int mem_mode) {
  char kw[9];
  if (ecl_block_has_kw(ecl_block , ecl_kw_get_header_ref(ecl_kw)))
    return false;
  else {
    list_node_type * list_node;
    switch(mem_mode) {
    case(COPY):
      list_node = list_append_copy(ecl_block->kw_list , ecl_kw , ecl_kw_copyc__ , ecl_kw_free__);
      break;
    case(OWNED_REF):
      list_node = list_append_list_owned_ref(ecl_block->kw_list , ecl_kw , ecl_kw_free__ );
      break;
    case(SHARED):
      list_node = list_append_ref(ecl_block->kw_list , ecl_kw);
      break;
    default:
      fprintf(stderr,"%s: internal programming error - aborting \n",__func__);
      abort();
    }
    
    util_set_strip_copy(kw , ecl_kw_get_header_ref(ecl_kw));
    hash_insert_ref(ecl_block->kw_hash , kw , list_node);
    ecl_block->size++;
    
    return true;
  }
}


bool ecl_block_has_kw(const ecl_block_type * ecl_block, const char * kw) {
  return hash_has_key(ecl_block->kw_hash , kw);
}



ecl_kw_type * ecl_block_get_first_kw(const ecl_block_type * src) {
  list_node_type *kw_node = list_get_head(src->kw_list);
  if (kw_node != NULL)
    return list_node_value_ptr(kw_node);
  else
    return NULL;
}
  

ecl_kw_type * ecl_block_get_next_kw(const ecl_block_type * ecl_block , const ecl_kw_type * ecl_kw) {
  char kw_s[9];
  list_node_type *kw_node;
  list_node_type *next_node;
  util_set_strip_copy(kw_s , ecl_kw_get_header_ref(ecl_kw));
  
  kw_node   = hash_get(ecl_block->kw_hash , kw_s);
  if (kw_node == NULL) {
    fprintf(stderr,"%s internal error in ecl_block.c - aborting \n",__func__);
    abort();
  } else {
    next_node = list_node_get_next(kw_node);
    if (next_node != NULL)
      return list_node_value_ptr(next_node);
    else
      return NULL;
  }
}



ecl_block_type * ecl_block_alloc_copy(const ecl_block_type *src) {
  ecl_block_type * copy;
  copy = ecl_block_alloc(src->report_nr , src->fmt_file , src->endian_convert);
  {
    list_node_type * kw_node = list_get_head(src->kw_list);
    while (kw_node != NULL) {
      const ecl_kw_type *kw = list_node_value_ptr(kw_node);
      ecl_block_add_kw(copy , kw , COPY);
      kw_node = list_node_get_next(kw_node);
    }
  }
    
  /*  
      {
      int i;
      char **kwlist = hash_alloc_ordered_keylist(src->kw_hash);
      for (i=0; i  < src->size; i++)
      ecl_block_add_kw_copy(copy , hash_get(src->kw_hash , kwlist[i]));
      hash_free_ext_keylist(src->kw_hash , kwlist);
      }
  */

  return copy;
}


void ecl_block_set_sim_time(ecl_block_type * block , time_t sim_time) {
  block->sim_time = sim_time;
}




void ecl_block_set_sim_time_restart(ecl_block_type * block) {
  int *date;
  ecl_kw_type *intehead_kw = ecl_block_get_kw(block , "INTEHEAD");
  
  if (intehead_kw == NULL) {
    fprintf(stderr,"%s: fatal error - could not locate INTEHEAD keyword in restart file - aborting \n",__func__);
    abort();
  }

  date = ecl_kw_iget_ptr(intehead_kw , 64);
  ecl_block_set_sim_time(block , util_make_time1(date[0] , date[1] , date[2]));
}



void ecl_block_set_sim_time_summary(ecl_block_type * block , /*int time_index , int years_index , */ int day_index , int month_index , int year_index) {
  float *date;
  ecl_kw_type * param_kw = ecl_block_get_kw(block , "PARAMS");
  date = ecl_kw_iget_ptr(param_kw , 0);


  {
    int sec  = 0;
    int min  = 0;
    int hour = 0;

    int day   = roundf(date[day_index]);
    int month = roundf(date[month_index]);
    int year  = roundf(date[year_index]);
    ecl_block_set_sim_time(block , util_make_time2(sec , min , hour , day , month , year));
  }
}





void ecl_block_set_report_nr(ecl_block_type * block , int report_nr) {
  block->report_nr      = report_nr;
}


int ecl_block_get_report_nr(const ecl_block_type * block) {
  return block->report_nr;
}

time_t ecl_block_get_sim_time(const ecl_block_type * block) {
  return block->sim_time;
}

ecl_block_type * ecl_block_alloc(int report_nr , bool fmt_file , bool endian_convert) {
  ecl_block_type *ecl_block;
  
  
  ecl_block = malloc(sizeof *ecl_block);
  ecl_block->fmt_file       = fmt_file;
  ecl_block->endian_convert = endian_convert;
  ecl_block->size           = 0;
  
  ecl_block->kw_hash  = hash_alloc(10);
  ecl_block->kw_list  = list_alloc();
  ecl_block->sim_time = -1;
  ecl_block_set_report_nr(ecl_block , report_nr);
  return ecl_block;
}


ecl_block_type * ecl_block_fread_alloc(int report_nr , bool fmt_file , bool endian_convert , fortio_type * fortio, bool *at_eof) {
  ecl_block_type * ecl_block = ecl_block_alloc(report_nr , fmt_file , endian_convert);
  ecl_block_fread(ecl_block , fortio , at_eof);
  return ecl_block;
}


ecl_kw_type * ecl_block_get_kw(const ecl_block_type *ecl_block , const char *kw) {
  ecl_kw_type *ecl_kw = NULL;
  char kw_s[9];
  util_set_strip_copy(kw_s , kw);  

  if (hash_has_key(ecl_block->kw_hash , kw_s)) 
    ecl_kw = list_node_value_ptr(hash_get(ecl_block->kw_hash , kw_s));
  else {
    fprintf(stderr,"%s: could not locate kw:%s in block - aborting. \n",__func__ , kw);
    ecl_block_summarize(ecl_block);
    abort();
  }
  return ecl_kw;
}




ecl_kw_type * ecl_block_detach_kw(ecl_block_type *ecl_block , const char *kw) {
  ecl_kw_type *ecl_kw = NULL;

  if (ecl_block_has_kw(ecl_block , kw)) {
    ecl_kw = ecl_block_get_kw(ecl_block , kw);
    list_node_type *kw_node = hash_get(ecl_block->kw_hash , kw);
    hash_del(ecl_block->kw_hash , kw);
    list_del_node(ecl_block->kw_list , kw_node);
  }
  
  return ecl_kw;
}



void ecl_block_free_kw(ecl_block_type *ecl_block , const char *kw) {
  ecl_kw_type *ecl_kw = ecl_block_detach_kw(ecl_block , kw);
  if (ecl_kw != NULL) 
    ecl_kw_free(ecl_kw);
}




bool ecl_block_fseek(int istep , bool fmt_file , bool abort_on_error , fortio_type * fortio) {
  if (istep == 0) 
    return true;
  else {
    ecl_kw_type *tmp_kw = ecl_kw_alloc_empty(fmt_file , fortio_get_endian_flip(fortio));     
    FILE *stream        = fortio_get_FILE(fortio);
    long int init_pos   = ftell(stream);
    char *first_kw;
    int   step_nr;
    bool block_found;
    step_nr = 1;
    
    if (ecl_kw_fread_header(tmp_kw , fortio)) {
      first_kw = util_alloc_string_copy(ecl_kw_get_header_ref(tmp_kw));
      block_found = true;
      do {
	block_found = ecl_kw_fseek_kw(first_kw , fmt_file , false , false , fortio);
	step_nr++;
      } while (block_found && (step_nr < istep));
    } else block_found = false;
    ecl_kw_free(tmp_kw);
    if (!block_found) {
      fseek(stream , init_pos , SEEK_SET);
      if (abort_on_error) {
	fprintf(stderr,"%s: failed to locate block number:%d - aborting \n",__func__ , istep);
	abort();
      }
    }
    return block_found;
  }
}



void ecl_block_fread(ecl_block_type *ecl_block, fortio_type *fortio , bool *at_eof) {
  ecl_kw_type *ecl_kw    = ecl_kw_alloc_empty(ecl_block->fmt_file , ecl_block->endian_convert);
  bool cont     = true;
  bool first_kw = true;
  

  while (cont) {
    if (ecl_kw_fread_realloc(ecl_kw , fortio)) {
      bool add_kw;

      /*
	This is *EXTREMELY UGLY* - when reading summary files we want
	to ensure that the SEQHDR keyword is the first header in any
	block (which contains the keyword).
	
	
      */

      if (ecl_kw_header_eq(ecl_kw , "SEQHDR") && !first_kw)
	add_kw = false;
      else 
	add_kw = ecl_block_add_kw(ecl_block , ecl_kw , COPY);
      
      if (!add_kw) {
	*at_eof = false;
	cont    = false;
	ecl_kw_rewind(ecl_kw , fortio);
      } 

    } else {
      cont    = false;
      *at_eof = true;
    }
    first_kw = false;
  }
  ecl_kw_free(ecl_kw);
}




void ecl_block_summarize(const ecl_block_type * block) {
  FILE * stream = stdout;
  list_node_type * kw_node = list_get_head(block->kw_list);
  fprintf(stream , "-----------------------------------------------------------------\n");
  while (kw_node != NULL) {
    ecl_kw_type *ecl_kw = list_node_value_ptr(kw_node);
    ecl_kw_summarize(ecl_kw);
    kw_node = list_node_get_next(kw_node);
  }
  fprintf(stream , "-----------------------------------------------------------------\n");
}


static bool ecl_block_include_kw(const ecl_block_type *ecl_block , const ecl_kw_type *ecl_kw , int N_kw, const char **kwlist) {
  const char *kw = ecl_kw_get_header_ref(ecl_kw);
  bool inc = false;
  int i;
  
  for (i=0; i < N_kw; i++) {
    if (strcmp(kwlist[i] , kw) == 0) {
      inc = true;
      break;
    }
  }
  return inc;
}


void ecl_block_set_fmt_file(ecl_block_type *ecl_block , bool fmt_file) {
  ecl_block->fmt_file = fmt_file;
  {
    /*
      This could be done with a for loop - it is a good stress test of
      the hash algorithm though.
    */
    hash_node_type *kw_node = hash_iter_init(ecl_block->kw_hash);
    while (kw_node != NULL) {
      ecl_kw_set_fmt_file(list_node_value_ptr(hash_node_value_ptr(kw_node)), ecl_block->fmt_file);
      kw_node = hash_iter_next(ecl_block->kw_hash , kw_node);
    }
  }
}


void ecl_block_select_formatted(ecl_block_type *ecl_block) { ecl_block_set_fmt_file(ecl_block , true ); }
void ecl_block_select_binary(ecl_block_type *ecl_block) { ecl_block_set_fmt_file(ecl_block , false); }


void ecl_block_fread_kwlist(ecl_block_type *ecl_block , fortio_type *fortio , int N_kw, const char **kwlist) {
  ecl_kw_type *ecl_kw  = ecl_kw_alloc_empty(ecl_block->fmt_file , ecl_block->endian_convert);
  hash_type   *kw_hash = hash_alloc(N_kw * 2);
  
  while (ecl_kw_fread_header(ecl_kw , fortio)) {
    if (ecl_block_include_kw(ecl_block, ecl_kw , N_kw , kwlist)) {
      ecl_kw_alloc_data(ecl_kw);
      ecl_kw_fread_data(ecl_kw , fortio);
      ecl_block_add_kw(ecl_block , ecl_kw , COPY);
    } else 
      ecl_kw_fskip_data(ecl_kw , fortio);
  }
  ecl_kw_free(ecl_kw);
  hash_free(kw_hash);
}


void ecl_block_fwrite(ecl_block_type *ecl_block , fortio_type *fortio) {
  list_node_type * kw_node = list_get_head(ecl_block->kw_list);
  while (kw_node != NULL) {
    ecl_kw_type *ecl_kw = list_node_value_ptr(kw_node);
    ecl_kw_set_fmt_file(ecl_kw , ecl_block->fmt_file);
    ecl_kw_fwrite(ecl_kw , fortio);
  }
}



void * ecl_block_get_data_ref(const ecl_block_type *ecl_block, const char *kw) {
  if (ecl_block != NULL) {
    if (ecl_block_has_kw(ecl_block , kw)) {
      ecl_kw_type *ecl_kw = ecl_block_get_kw(ecl_block , kw);
      return ecl_kw_get_data_ref(ecl_kw);
    } else
      return NULL; 
  } else
    return NULL;
}



void ecl_block_free(ecl_block_type *ecl_block) {
  hash_free(ecl_block->kw_hash);
  list_free(ecl_block->kw_list);
  free(ecl_block);
}


