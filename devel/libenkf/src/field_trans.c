/*
  This file implements a number of functions used for init and output
  transformations of fields. The prototype for these functions is very
  simple: "one float in - one float out". 

  It is mainly implemented in this file, so that it will be easy to
  adde new transformation functions without diving into the the full
  field / field_config complexity.

  Documentation on how to add a new transformation function is at the
  bottom of the file.
*/
#include <string.h>
#include <field_trans.h>
#include <hash.h>
#include <stdbool.h>
#include <util.h>
#include <math.h>

/*****************************************************************/

struct field_trans_table_struct {
  bool        case_sensitive;
  hash_type * function_table;
};



typedef struct {
  char 		  * key;
  char 		  * description;
  field_func_type * func;
} field_func_node_type;

/*****************************************************************/

static field_func_node_type * field_func_node_alloc(const char * key , const char * description , field_func_type * func) {
  field_func_node_type * node = util_malloc( sizeof * node , __func__);

  node->key         = util_alloc_string_copy( key );
  node->description = util_alloc_string_copy( description );
  node->func        = func;
  
  return node;
}


static void field_func_node_free(field_func_node_type * node) {
  free(node->key);
  util_safe_free( node->description );
  free(node);
}


static void field_func_node_free__(void * node) {
  field_func_node_free( (field_func_node_type *) node);
}


static void field_func_node_fprintf(const field_func_node_type * node , FILE * stream) {
  if (node->description != NULL)
    fprintf(stream , "%16s: %s \n",node->key , node->description);
  else
    fprintf(stream , "%16s: No description \n",node->key  );
}





/*****************************************************************/

void field_trans_table_add(field_trans_table_type * table , const char * _key , const char * description , field_func_type * func) {
  char * key;
  
  if (table->case_sensitive)
    key = util_alloc_string_copy( _key );
  else
    key = util_alloc_strupr_copy( _key );

  {
    field_func_node_type * node = field_func_node_alloc( key , description , func );
    hash_insert_hash_owned_ref(table->function_table , key , node , field_func_node_free__);
  }
  free(key);
}



void field_trans_table_fprintf(const field_trans_table_type * table , FILE * stream) {
  hash_iter_type * iter = hash_iter_alloc(table->function_table);
  const char * key = hash_iter_get_next_key(iter); 
  fprintf(stream,"==========================================================================================\n");
  while (key != NULL) {
    field_func_node_type * func_node = hash_get(table->function_table , key);
    field_func_node_fprintf(func_node , stream);
    key = hash_iter_get_next_key(iter);
  }
  fprintf(stream,"==========================================================================================\n");
  hash_iter_free(iter);
}



/*
  This function takes a key input, and returns a pointer to the
  corresponding function. The function will fail if the key is not
  recognized.
*/


field_func_type * field_trans_table_lookup(field_trans_table_type * table , const char * _key) {
  field_func_type * func;
  char * key;

  if (table->case_sensitive)
    key = util_alloc_string_copy(_key);
  else
    key = util_alloc_strupr_copy(_key);

  if (hash_has_key(table->function_table , key)) {
    field_func_node_type * func_node = hash_get(table->function_table , key);
    func = func_node->func;
  } else {
    fprintf(stderr , "Sorry: the field transformation function:%s is not recognized \n\n",key);
    field_trans_table_fprintf(table , stderr);
    util_exit("Exiting ... \n");
    func = NULL; /* Compiler shut up. */
  }
  free( key );
  return func;
}



bool field_trans_table_has_key(field_trans_table_type * table , const char * _key) {
  bool has_key;
  char * key;
  if (table->case_sensitive)
    key = util_alloc_string_copy(_key);
  else
    key = util_alloc_strupr_copy(_key);

  has_key = hash_has_key( table->function_table , key);
  free(key);
  
  return has_key;
}


void field_trans_table_free(field_trans_table_type * table ) {
  hash_free( table->function_table );
  free( table );
}




/*****************************************************************/
/*****************************************************************/
/* Here comes the actual functions. To add a new function:       */
/*                                                               */
/*  1. Write the function - as a float in - float out.           */
/*  2. Register the function in field_trans_table_alloc().       */
/*                                                               */ 
/*****************************************************************/
 



static float field_trans_pow10(float x) {
  return powf(10.0 , x);
}


static float trunc_pow10f(float x) {
  return util_float_max(powf(10.0 , x) , 0.001);
}


field_trans_table_type * field_trans_table_alloc() {
  field_trans_table_type * table = util_malloc( sizeof * table , __func__);
  table->function_table = hash_alloc();
  field_trans_table_add( table , "POW10"       , "This function will raise x to the power of 10: y = 10^x." ,                            field_trans_pow10);
  field_trans_table_add( table , "TRUNC_POW10" , "This function will raise x to the power of 10 - and truncate lower values at 0.001." , trunc_pow10f);
  field_trans_table_add( table , "LOG"         , "This function will take the NATURAL logarithm of x: y = ln(x)" , logf);
  field_trans_table_add( table , "LN"          , "This function will take the NATURAL logarithm of x: y = ln(x)" , logf);
  field_trans_table_add( table , "LOG10"       , "This function will take the log10 logarithm of x: y = log10(x)" , log10f);
  table->case_sensitive = false;
  return table;
}

