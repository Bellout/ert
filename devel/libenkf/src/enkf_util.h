#ifndef __ENKF_UTIL_H__
#define __ENKF_UTIL_H__
#ifdef __cplusplus
extern "C" {
#endif
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <enkf_types.h>
#include <ecl_util.h>
#include <buffer.h>

/*****************************************************************/

/**
   These math functions assume that the underlying data implementation
   is double or float - will NOT work for fields. 
*/



#define SQR_FUNC(prefix)                            \
void prefix ## _isqr(void * void_arg) {             \
prefix ## _type *arg = (prefix ## _type *) void_arg;\
const int data_size = prefix ## _config_get_data_size(arg->config);  \
int i;                                              \
for (i=0; i < data_size; i++)                       \
 arg->data[i] = arg->data[i]*arg->data[i];          \
}
#define SQR_FUNC_VOID_HEADER(prefix) void prefix ## _isqr(void * )
#define SQR_FUNC_HEADER(prefix) void prefix ## _isqr(prefix ## _type *)
#define SQR_FUNC_SCALAR(prefix) void prefix ## _isqr(void * void_arg) { scalar_isqr(((prefix ## _type *) void_arg)->scalar); }


/*****************************************************************/

#define SQRT_FUNC(prefix)                           \
void prefix ## _isqrt(void * void_arg) {            \
prefix ## _type *arg = (prefix ## _type *) void_arg;\
const int data_size = prefix ## _config_get_data_size(arg->config);  \
int i;                                              \
for (i=0; i < data_size; i++)                       \
 arg->data[i] = sqrt(arg->data[i]);                 \
}
#define SQRT_FUNC_VOID_HEADER(prefix) void prefix ## _isqrt(void * )
#define SQRT_FUNC_HEADER(prefix) void prefix ## _isqrt(prefix ## _type* )
#define SQRT_FUNC_SCALAR(prefix)   void prefix ## _isqrt(void * void_arg) { scalar_isqrt(((prefix ## _type *) void_arg)->scalar); }


/*****************************************************************/

#define SCALE_FUNC(prefix)                                                 \
void prefix ## _iscale(void *void_arg , double scale_factor) {             \
prefix ## _type *arg = (prefix ## _type *) void_arg;                       \
const int data_size = prefix ## _config_get_data_size(arg->config);        \
int i;                                              			   \
for (i=0; i < data_size; i++)                            		   \
 arg->data[i] *= scale_factor;                  			   \
}
#define SCALE_FUNC_SCALAR(prefix)   void prefix ## _iscale(void * void_arg, double scale_factor) { scalar_iscale(((prefix ## _type *) void_arg)->scalar , scale_factor); }
#define SCALE_FUNC_VOID_HEADER(prefix) void prefix ## _iscale(void * , double)
#define SCALE_FUNC_HEADER(prefix) void prefix ## _iscale(prefix ## _type * , double)

/*****************************************************************/

#define RESET_FUNC(prefix)                                                 \
void prefix ## _ireset(void *void_arg) {             \
prefix ## _type *arg = (prefix ## _type *) void_arg;                       \
const int data_size = prefix ## _config_get_data_size(arg->config);        \
int i;                                              			   \
for (i=0; i < data_size; i++)                                              \
 arg->data[i] = 0;                               			   \
}
#define RESET_FUNC_SCALAR(prefix)   void prefix ## _ireset(void * void_arg) { scalar_ireset(((prefix ## _type *) void_arg)->scalar); }
#define RESET_FUNC_VOID_HEADER(prefix) void prefix ## _ireset(void *)
#define RESET_FUNC_HEADER(prefix) void prefix ## _ireset(prefix ## _type *)

/*****************************************************************/

#define ADD_FUNC(prefix)                                                       \
void prefix ## _iadd(void *void_arg , const void *void_delta) {                \
      prefix ## _type *arg   = (prefix ## _type *)       void_arg;  	       \
const prefix ## _type *delta = (const prefix ## _type *) void_delta;	       \
const prefix ## _config_type *config = arg->config; 			       \
const int data_size = prefix ## _config_get_data_size(arg->config);            \
int i;                                              			       \
if (config != delta->config) {                                                 \
    fprintf(stderr,"%s:two object have different config objects - aborting \n",__func__);\
    abort();                                                                   \
}                                                                              \
for (i=0; i < data_size; i++)                            			       \
 arg->data[i] += delta->data[i];                                               \
}

#define ADD_FUNC_SCALAR(prefix)   void prefix ## _iadd(void *void_arg,  const void * void_factor) { scalar_iadd( ((prefix ## _type *) void_arg)->scalar , (( const prefix ## _type *) void_factor)->scalar); }
#define ADD_FUNC_VOID_HEADER(prefix) void prefix ## _iadd(void * , const void *)
#define ADD_FUNC_HEADER(prefix) void prefix ## _iadd(prefix ## _type * , const prefix ## _type *)
/*****************************************************************/

#define MUL_ADD_FUNC(prefix)                                                                         \
void prefix ## _imul_add(void *void_arg , double scale_factor , const void *void_delta) {                \
      prefix ## _type *arg   = (prefix ## _type *)       void_arg;  	       \
const prefix ## _type *delta = (const prefix ## _type *) void_delta;	       \
const prefix ## _config_type *config = arg->config; 			       \
const int data_size = prefix ## _config_get_data_size(arg->config);            \
int i;                                              			       \
if (config != delta->config) {                                                 \
    fprintf(stderr,"%s:two object have different config objects - aborting \n",__func__);\
    abort();                                                                   \
}                                                                              \
for (i=0; i < data_size; i++)                            			       \
 arg->data[i] += scale_factor * delta->data[i];                                        \
}

#define MUL_ADD_FUNC_SCALAR(prefix)   void prefix ## _imul_add(void *void_arg,  double scale_factor , const void * void_factor) { scalar_imul_add( ((prefix ## _type *) void_arg)->scalar , scale_factor , (( const prefix ## _type *) void_factor)->scalar); }
#define MUL_ADD_FUNC_VOID_HEADER(prefix) void prefix ## _imul_add(void * , double , const void *)
#define MUL_ADD_FUNC_HEADER(prefix) void prefix ## _imul_add(prefix ## _type* , double , const prefix ## _type *)

/*****************************************************************/

#define SUB_FUNC(prefix)                                                       \
void prefix ## _isub(void *void_arg , const void *void_diff) {                 \
      prefix ## _type *arg   = (prefix ## _type *)       void_arg;  	       \
const prefix ## _type *diff = (const prefix ## _type *) void_diff;	       \
const prefix ## _config_type *config = arg->config; 			       \
const int data_size = prefix ## _config_get_data_size(arg->config);            \
int i;                                              			       \
if (config != diff->config) {                                                  \
    fprintf(stderr,"%s:two object have different config objects - aborting \n",__func__);\
    abort();                                                                   \
}                                                                              \
for (i=0; i < data_size; i++)                            		       \
 arg->data[i] -= diff->data[i];                                                \
}

#define SUB_FUNC_SCALAR(prefix)   void prefix ## _isub(void *void_arg,  const void * void_factor) { scalar_isub( ((prefix ## _type *) void_arg)->scalar , (( const prefix ## _type *) void_factor)->scalar); }
#define SUB_FUNC_VOID_HEADER(prefix) void prefix ## _isub(void * , const void *)
#define SUB_FUNC_HEADER(prefix) void prefix ## _isub(prefix ## _type * , const prefix ## _type*)
/*****************************************************************/

#define MUL_FUNC(prefix)                                                       \
void prefix ## _imul(void *void_arg , const void *void_factor) {                \
      prefix ## _type *arg    = (prefix ## _type *)       void_arg;  	       \
const prefix ## _type *factor = (const prefix ## _type *) void_factor;	       \
const prefix ## _config_type *config = arg->config; 			       \
const int data_size = prefix ## _config_get_data_size(arg->config);            \
int i;                                              			       \
if (config != factor->config) {                                                 \
    fprintf(stderr,"%s:two object have different config objects - aborting \n",__func__);\
    abort();                                                                   \
}                                                                              \
for (i=0; i < data_size; i++)                            			       \
 arg->data[i] *= factor->data[i];                                               \
}
#define MUL_FUNC_SCALAR(prefix)   void prefix ## _imul(void *void_arg,  const void * void_factor) { scalar_imul( ((prefix ## _type *) void_arg)->scalar , (( const prefix ## _type *) void_factor)->scalar); }
#define MUL_FUNC_VOID_HEADER(prefix) void prefix ## _imul(void * , const void *)
#define MUL_FUNC_HEADER(prefix) void prefix ## _imul(prefix ## _type * , const prefix ## _type*)

/*****************************************************************/

#define ADDSQR_FUNC(prefix)                                                       \
void prefix ## _iaddsqr(void *void_arg , const void *void_delta) {                \
      prefix ## _type *arg   = (prefix ## _type *)       void_arg;  	       \
const prefix ## _type *delta = (const prefix ## _type *) void_delta;	       \
const prefix ## _config_type *config = arg->config; 			       \
const int data_size = prefix ## _config_get_data_size(arg->config);            \
int i;                                              			       \
if (config != delta->config) {                                                 \
    fprintf(stderr,"%s:two object have different config objects - aborting \n",__func__);\
    abort();                                                                   \
}                                                                              \
for (i=0; i < data_size; i++)                            		       \
 arg->data[i] += delta->data[i] * delta->data[i];                              \
}
#define ADDSQR_FUNC_SCALAR(prefix)   void prefix ## _iaddsqr(void *void_arg,  const void * void_factor) { scalar_iaddsqr( ((prefix ## _type *) void_arg)->scalar , (( const prefix ## _type *) void_factor)->scalar); }
#define ADDSQR_FUNC_VOID_HEADER(prefix) void prefix ## _iaddsqr(void * , const void *)
#define ADDSQR_FUNC_HEADER(prefix) void prefix ## _iaddsqr(prefix ## _type * , const prefix ## _type *)



#define MATH_OPS(prefix) \
SQR_FUNC    (prefix) \
SQRT_FUNC   (prefix) \
SCALE_FUNC  (prefix) \
ADD_FUNC    (prefix) \
ADDSQR_FUNC (prefix) \
SUB_FUNC    (prefix) \
MUL_FUNC    (prefix) \
MUL_ADD_FUNC(prefix) 

#define MATH_OPS_SCALAR(prefix) \
SQR_FUNC_SCALAR    (prefix) \
SQRT_FUNC_SCALAR   (prefix) \
SCALE_FUNC_SCALAR  (prefix) \
ADD_FUNC_SCALAR    (prefix) \
ADDSQR_FUNC_SCALAR (prefix) \
SUB_FUNC_SCALAR    (prefix) \
MUL_FUNC_SCALAR    (prefix) \
MUL_ADD_FUNC_SCALAR(prefix) 



#define MATH_OPS_VOID_HEADER(prefix) \
SQR_FUNC_VOID_HEADER    (prefix);  \
SQRT_FUNC_VOID_HEADER   (prefix);  \
SCALE_FUNC_VOID_HEADER  (prefix);  \
ADD_FUNC_VOID_HEADER    (prefix);  \
ADDSQR_FUNC_VOID_HEADER (prefix);  \
SUB_FUNC_VOID_HEADER    (prefix);  \
MUL_FUNC_VOID_HEADER    (prefix);  \
MUL_ADD_FUNC_VOID_HEADER(prefix)

#define MATH_OPS_HEADER(prefix) \
SQR_FUNC_HEADER    (prefix);  \
SQRT_FUNC_HEADER   (prefix);  \
SCALE_FUNC_HEADER  (prefix);  \
ADD_FUNC_HEADER    (prefix);  \
ADDSQR_FUNC_HEADER (prefix);  \
SUB_FUNC_HEADER    (prefix);  \
MUL_FUNC_HEADER    (prefix);  \
MUL_ADD_FUNC_HEADER(prefix)

/*****************************************************************/

#define ENSEMBLE_MULX_VECTOR(prefix) \
void prefix ## _ensemble_mulX_vector(prefix ## _type *new , int ens_size , const prefix ## _type ** prefix ## _ensemble , const double *X_vector) { \
  int iens;              \
  prefix ## _clear(new); \
  for (iens=0; iens < ens_size; iens++)  \
     prefix ## _imul_add(new , X_vector[iens] , prefix ## _ensemble[iens]);\
}
#define ENSEMBLE_MULX_VECTOR_HEADER(prefix) void prefix ## _ensemble_mulX_vector(prefix ## _type *, int , const prefix ## _type ** , const double *);


#define ENSEMBLE_MULX_VECTOR_VOID(prefix) \
void prefix ## _ensemble_mulX_vector__(void *new , int ens_size , const void ** prefix ## _ensemble , const double *X_vector) { \
   prefix ## _ensemble_mulX_vector((prefix ## _type *) new , ens_size , (const prefix ## _type **) prefix ## _ensemble , X_vector); \
} 

#define ENSEMBLE_MULX_VECTOR_VOID_HEADER(prefix) void prefix ## _ensemble_mulX_vector__(void * , int  , const void ** , const  double *);

/*****************************************************************/

#define ALLOC_STATS_SCALAR(prefix)                                                                                                   \
void prefix ## _alloc_stats(const prefix ## _type ** ensemble , int ens_size , prefix ## _type ** _mean , prefix ## _type ** _std) { \
  int iens;                                                                                                                          \
  prefix ## _type * mean = prefix ## _copyc(ensemble[0]);									     \
  prefix ## _type * std  = prefix ## _copyc(ensemble[0]);									     \
  prefix ## _clear(mean);				 									     \
  prefix ## _clear(std); 				 									     \
  for (iens = 0; iens < ens_size; iens++) {     	 									     \
    prefix ## _output_transform(ensemble[iens]);	 									     \
    prefix ## _iadd(mean   , ensemble[iens]);   	 									     \
    prefix ## _iaddsqr(std , ensemble[iens]);   	 									     \
  }                                        		 									     \
  prefix ## _iscale(mean , 1.0 / ens_size);		 									     \
  prefix ## _iscale(std  , 1.0 / ens_size);		 									     \
  {                                        		 									     \
    prefix ## _type * tmp = prefix ## _copyc(mean);	 									     \
    prefix ## _isqr(tmp);                          	 									     \
    prefix ## _imul_add(std , -1.0 , tmp);	   	 									     \
    prefix ## _free(tmp);                 	   	 									     \
  }                                       	   	 									     \
  prefix ## _isqrt(std);                           	 									     \
  *_mean = mean;				   	 									     \
  *_std  = std; 				   	 									     \
}




#define ALLOC_STATS(prefix)                                                                                                          \
void prefix ## _alloc_stats(const prefix ## _type ** ensemble , int ens_size , prefix ## _type ** _mean , prefix ## _type ** _std) { \
  int iens;                                                                                                                          \
  prefix ## _type * mean = prefix ## _copyc(ensemble[0]);									     \
  prefix ## _type * std  = prefix ## _copyc(ensemble[0]);									     \
  prefix ## _clear(mean);				 									     \
  prefix ## _clear(std); 				 									     \
  for (iens = 0; iens < ens_size; iens++) {     	 									     \
    prefix ## _iadd(mean   , ensemble[iens]);   	 									     \
    prefix ## _iaddsqr(std , ensemble[iens]);   	 									     \
  }                                        		 									     \
  prefix ## _iscale(mean , 1.0 / ens_size);		 									     \
  prefix ## _iscale(std  , 1.0 / ens_size);		 									     \
  {                                        		 									     \
    prefix ## _type * tmp = prefix ## _copyc(mean);	 									     \
    prefix ## _isqr(tmp);                          	 									     \
    prefix ## _imul_add(std , -1.0 , tmp);	   	 									     \
    prefix ## _free(tmp);                 	   	 									     \
  }                                       	   	 									     \
  prefix ## _isqrt(std);                           	 									     \
  if (_mean != NULL)   *_mean = mean;				   	 							     \
  if (_std != NULL)    *_std  = std; 				   	 							     \
}



#define ALLOC_STATS_HEADER(prefix) void prefix ## _alloc_stats(const prefix ## _type ** , int , prefix ## _type ** , prefix ## _type ** );
/*****************************************************************/


double enkf_util_random_uniform();
int    enkf_util_random_int();

void    enkf_util_truncate(void *  , int  , ecl_type_enum  , void *  , void *);
void 	enkf_util_rand_stdnormal_vector(int  , double *);
double 	enkf_util_rand_normal(double , double );
void   	enkf_util_fwrite_target_type(FILE * , enkf_impl_type);
void    enkf_util_assert_buffer_type(buffer_type * buffer, enkf_impl_type target_type);

char  * enkf_util_scanf_alloc_filename(const char * , int );
void    enkf_util_fprintf_data(const int * , const double ** , const char * , const char ** , int , int , const bool * , bool , FILE * stream);

char * enkf_util_alloc_tagged_string(const char * );
int    enkf_util_compare_keys( const char * key1 , const char * key2 );
int    enkf_util_compare_keys__( const void * __key1 , const void * __key2 );

/* These #defines are used in the enkf_util_scanf_alloc_filename function. */
#define EXISTING_FILE  1
#define NEW_FILE       2
#define AUTO_MKDIR     4


#ifdef __cplusplus
}
#endif
#endif
