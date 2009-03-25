#ifndef __SET_H__
#define __SET_H__
#ifdef __cplusplus
extern "C" {
#endif
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

#include <hash.h>

typedef struct set_struct set_type;
typedef struct set_iter_struct set_iter_type;

void         set_remove_key(set_type * , const char * );
set_type   * set_alloc(int , const char ** );
set_type   * set_alloc_empty();
bool         set_add_key(set_type * , const char * );
bool         set_has_key(const set_type * set, const char * );
void         set_free(set_type * );
int          set_get_size(const set_type *);
char      ** set_alloc_keylist(const set_type * );
void         set_fwrite(const set_type * , FILE * );
void         set_fread(set_type * , FILE * );
set_type   * set_fread_alloc(FILE *);
void         set_fprintf(const set_type * , FILE * );
void         set_intersect(set_type * , const set_type * );
void         set_union(set_type * , const set_type * );
void         set_minus(set_type * , const set_type * );
set_type   * set_copyc(const set_type *);


set_iter_type * set_iter_alloc(const set_type * set);
void set_iter_free(set_iter_type * set_iter);
bool set_iter_is_complete(const set_iter_type * set_iter);
const char * set_iter_get_next_key(set_iter_type * set_iter);


#ifdef __cplusplus
}
#endif
#endif
