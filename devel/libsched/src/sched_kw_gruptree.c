#include <hash.h>
#include <util.h>
#include <sched_kw_gruptree.h>
#include <sched_util.h>
#include <sched_macros.h>
#include <stringlist.h>

struct sched_kw_gruptree_struct
{
  hash_type * gruptree_hash;
};



/***********************************************************************/



static void sched_kw_gruptree_add_line(sched_kw_gruptree_type * kw, const char * line)
{
  int tokens;
  char **token_list;

  sched_util_parse_line(line, &tokens, &token_list, 2, NULL);

  if(tokens > 2)
    util_abort("%s: Error when parsing record in GRUPTREE. Record must have one or two strings. Found %i - aborting.\n",__func__,tokens);
  
  if(token_list[1] == NULL)
    hash_insert_string(kw->gruptree_hash, token_list[0], "FIELD");
  else
    hash_insert_string(kw->gruptree_hash, token_list[0], token_list[1]);

  util_free_stringlist( token_list , tokens );
};



static sched_kw_gruptree_type * sched_kw_gruptree_alloc()
{
  sched_kw_gruptree_type * kw = util_malloc(sizeof * kw, __func__);
  kw->gruptree_hash = hash_alloc();
  
  return kw;
};



/***********************************************************************/


sched_kw_gruptree_type * sched_kw_gruptree_token_alloc(const stringlist_type * tokens , int * __token_index ) {
  
}



sched_kw_gruptree_type * sched_kw_gruptree_fscanf_alloc(FILE * stream, bool * at_eof, const char * kw_name)
{
  bool   at_eokw = false;
  char * line;
  sched_kw_gruptree_type * kw = sched_kw_gruptree_alloc();

  while(!*at_eof && !at_eokw)
  {
    line = sched_util_alloc_next_entry(stream, at_eof, &at_eokw);
    if(at_eokw)
    {
      break;
    }
    else if(*at_eof)
    {
      util_abort("%s: Reached EOF before GRUPTREE was finished - aborting.\n", __func__);
    }
    else
    {
      sched_kw_gruptree_add_line(kw, line);
      free(line);
    }
  }

  return kw;
}





void sched_kw_gruptree_free(sched_kw_gruptree_type * kw)
{
  hash_free(kw->gruptree_hash);
  free(kw);
};


void sched_kw_gruptree_fprintf(const sched_kw_gruptree_type * kw, FILE * stream)
{

  fprintf(stream, "GRUPTREE\n");
  
  {
    const int   num_keys = hash_get_size(kw->gruptree_hash);
    char ** child_list   = hash_alloc_keylist(kw->gruptree_hash);
    int i;

    for (i = 0; i < num_keys; i++) {
      const char * parent_name = hash_get_string(kw->gruptree_hash , child_list[i]);
      fprintf(stream,"  '%s'  '%s' /\n",child_list[i] , parent_name);
    }
    util_free_stringlist( child_list , num_keys );
  }

  fprintf(stream,"/\n\n");
};



void sched_kw_gruptree_fwrite(const sched_kw_gruptree_type * kw, FILE * stream)
{
  int gruptree_lines = hash_get_size(kw->gruptree_hash);
  util_fwrite(&gruptree_lines, sizeof gruptree_lines, 1, stream, __func__);
  {
    const int   num_keys = hash_get_size(kw->gruptree_hash);
    char ** child_list   = hash_alloc_keylist(kw->gruptree_hash);
    int i;

    for (i = 0; i < num_keys; i++) {
      const char * parent_name = hash_get_string(kw->gruptree_hash , child_list[i]);

      util_fwrite_string(child_list[i] , stream);
      util_fwrite_string(parent_name   , stream);
    }
    util_free_stringlist( child_list , num_keys );
  }
}



sched_kw_gruptree_type * sched_kw_gruptree_fread_alloc(FILE * stream)
{
  int i, gruptree_lines;
  char * child_name;
  char * parent_name;

  sched_kw_gruptree_type * kw = sched_kw_gruptree_alloc();

  util_fread(&gruptree_lines, sizeof gruptree_lines, 1, stream, __func__);

  for(i=0; i<gruptree_lines; i++)
  {
    child_name  = util_fread_alloc_string(stream);
    parent_name = util_fread_alloc_string(stream);
    hash_insert_string(kw->gruptree_hash,child_name,parent_name);
    free(child_name);
    free(parent_name);
  }

  return kw;
};



void sched_kw_gruptree_alloc_child_parent_list(const sched_kw_gruptree_type * kw, char *** __children, char *** __parents, int * num_pairs)
{
  *num_pairs = hash_get_size(kw->gruptree_hash);
  char ** children = hash_alloc_keylist(kw->gruptree_hash);
  char ** parents = util_malloc(*num_pairs * sizeof * parents, __func__);

  for(int child_nr = 0; child_nr < *num_pairs; child_nr++)
  {
    parents[child_nr] = util_alloc_string_copy(hash_get_string(kw->gruptree_hash, children[child_nr]));
  }

  *__children = children;
  *__parents  = parents;
}


sched_kw_gruptree_type * sched_kw_gruptree_alloc_copy(const sched_kw_gruptree_type * src) {
  sched_kw_gruptree_type * target = sched_kw_gruptree_alloc();
  hash_iter_type * iter = hash_iter_alloc(src->gruptree_hash);
  const char * kw = hash_iter_get_next_key(iter);
  while (kw != NULL) {
    char * parent_name = hash_get_string(src->gruptree_hash , kw);
    hash_insert_string( target->gruptree_hash , kw , parent_name);
    kw = hash_iter_get_next_key(iter);
  }
  hash_iter_free(iter);
  return target;
}



/***********************************************************************/

KW_IMPL(gruptree)
KW_ALLOC_COPY_IMPL(gruptree)
