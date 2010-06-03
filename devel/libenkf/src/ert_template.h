#ifndef __ERT_TEMPLATE_H__
#define __ERT_TEMPLATE_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <subst_list.h>
#include <stringlist.h>

typedef struct ert_template_struct  ert_template_type;
typedef struct ert_templates_struct ert_templates_type;


stringlist_type   * ert_templates_alloc_list( ert_templates_type * ert_templates);
ert_template_type * ert_template_alloc( const char * template_file , const char * target_file, subst_list_type * parent_subst) ;
void                ert_template_free( ert_template_type * ert_tamplete );
void                ert_template_instantiate( ert_template_type * ert_template , const char * path , const subst_list_type * arg_list ); 
void                ert_template_add_arg( ert_template_type * ert_template , const char * key , const char * value );
void                ert_template_free__(void * arg);

void                ert_templates_clear( ert_templates_type * ert_templates );
ert_template_type * ert_templates_get_template( ert_templates_type * ert_templates , const char * key);

ert_templates_type * ert_templates_alloc(subst_list_type * parent_subst);
void                 ert_templates_free( ert_templates_type * ert_templates );
ert_template_type  * ert_templates_add_template( ert_templates_type * ert_templates , const char * key , const char * template_file , const char * target_file , const char * arg_string);
void                 ert_templates_instansiate( ert_templates_type * ert_templates , const char * path , const subst_list_type * arg_list);
void                 ert_templates_del_template( ert_templates_type * ert_templates , const char * key);

const char         * ert_template_get_template_file( const ert_template_type * ert_template);
const char         * ert_template_get_target_file( const ert_template_type * ert_template);
const char         * ert_template_get_args_as_string( const ert_template_type * ert_template );

#ifdef __cplusplus
}
#endif
#endif
