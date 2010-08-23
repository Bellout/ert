#ifndef __STRINGLIST_H__
#define __STRINGLIST_H__
#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <stdio.h>
#include <int_vector.h>
#include <buffer.h>

typedef struct stringlist_struct stringlist_type;
typedef int  ( string_cmp_ftype)  (const void * , const void *);


void              stringlist_deep_copy( stringlist_type * target , const stringlist_type * src);

stringlist_type * stringlist_alloc_new();
void              stringlist_free__(void * );
void              stringlist_free(stringlist_type *);
void              stringlist_clear(stringlist_type * );

void              stringlist_append_copy(stringlist_type * , const char *);
void              stringlist_append_ref(stringlist_type * , const char *);
void              stringlist_append_owned_ref(stringlist_type * , const char *);

const      char * stringlist_safe_iget( const stringlist_type * stringlist , int index);
bool              stringlist_iequal( const stringlist_type * stringlist , int index, const char * s );
const      char * stringlist_iget(const stringlist_type * , int);
char            * stringlist_iget_copy(const stringlist_type * stringlist , int );
char            * stringlist_alloc_joined_string(const stringlist_type *  , const char * );
char            * stringlist_alloc_joined_segment_string( const stringlist_type * s , int start_index , int end_index , const char * sep );

void 		  stringlist_iset_copy(stringlist_type *, int index , const char *);
void 		  stringlist_iset_ref(stringlist_type *, int index , const char *);
void 		  stringlist_iset_owned_ref(stringlist_type *, int index , const char *);

void 		  stringlist_insert_copy(stringlist_type *, int index , const char *);
void 		  stringlist_insert_ref(stringlist_type *, int index , const char *);
void 		  stringlist_insert_owned_ref(stringlist_type *, int index , const char *);

void              stringlist_idel(stringlist_type * stringlist , int index);

int               stringlist_get_size(const stringlist_type * );
void              stringlist_fprintf(const stringlist_type * , const char * , FILE *);
void              stringlist_fprintf_fmt(const stringlist_type * stringlist, const stringlist_type * fmt_list , FILE * stream);


stringlist_type * stringlist_alloc_argv_copy(const char **      , int );
stringlist_type * stringlist_alloc_argv_ref (const char **      , int );
stringlist_type * stringlist_alloc_argv_owned_ref(const char ** argv , int argc);
stringlist_type * stringlist_alloc_shallow_copy(const stringlist_type *);
stringlist_type * stringlist_alloc_deep_copy(const stringlist_type *);
stringlist_type * stringlist_alloc_shallow_copy_with_offset(const stringlist_type * stringlist, int offset);
stringlist_type * stringlist_alloc_shallow_copy_with_limits(const stringlist_type * stringlist, int start, int num_strings);
stringlist_type * stringlist_alloc_from_split( const char * input_string , const char * sep );
stringlist_type * stringlist_fread_alloc(FILE * ); 

void              stringlist_append_stringlist_copy(stringlist_type *  , const stringlist_type * );
void              stringlist_append_stringlist_ref(stringlist_type *   , const stringlist_type * );
void              stringlist_insert_stringlist_copy(stringlist_type *  , const stringlist_type *, int);

bool              stringlist_equal(const stringlist_type *  , const stringlist_type *);
bool              stringlist_contains(const stringlist_type *  , const char * );
int_vector_type * stringlist_find(const stringlist_type *, const char *);
int               stringlist_find_first(const stringlist_type * , const char * );
int   	          stringlist_get_argc(const stringlist_type * );
char           ** stringlist_alloc_char_copy(const stringlist_type * );
void              stringlist_fread(stringlist_type * , FILE * );
void              stringlist_fwrite(const stringlist_type * , FILE * );
void              stringlist_buffer_fread( stringlist_type * s , buffer_type * buffer );
void              stringlist_buffer_fwrite( const stringlist_type * s , buffer_type * buffer );
void              stringlist_sort(stringlist_type * , string_cmp_ftype * string_cmp);

int               stringlist_select_matching(stringlist_type * names , const char * pattern);
UTIL_IS_INSTANCE_HEADER(stringlist);

#ifdef __cplusplus
}
#endif
#endif 
