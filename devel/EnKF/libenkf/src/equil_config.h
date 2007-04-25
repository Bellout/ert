#ifndef __EQUIL_CONFIG_H__
#define __EQUIL_CONFIG_H__

#include <stdio.h>
#include <stdbool.h>
#include <enkf_util.h>
#include <config.h>

typedef struct equil_config_struct equil_config_type;

struct equil_config_struct {
  CONFIG_STD_FIELDS;
  double * mean;
  double * std;
  bool   * active;
};


equil_config_type   * equil_config_alloc(int , const char * , const char * );
void                  equil_config_free(equil_config_type *);
const          char * equil_config_get_ensname_ref(const equil_config_type * );
const          char * equil_config_get_eclname_ref(const equil_config_type * );
int                   equil_config_get_nequil(const equil_config_type *);

CONFIG_GET_SIZE_FUNC_HEADER(equil);
CONFIG_SET_ECL_FILE_HEADER_VOID(equil);
CONFIG_SET_ENS_FILE_HEADER_VOID(equil);
CONFIG_GET_SIZE_FUNC_HEADER(equil);

VOID_FUNC_HEADER(equil_config_free);

#endif
