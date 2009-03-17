#ifndef __CONF_DATA_H__
#define __CONF_DATA_H__
#include <stdbool.h>
#include <time.h>
#include <int_vector.h>
#include <double_vector.h>
#include <time_t_vector.h>

#define DT_VECTOR_SEP " \t\r\n,"

typedef enum {
              DT_STR,
              DT_INT,
              DT_POSINT,
              DT_FLOAT,
              DT_POSFLOAT,
              DT_INT_VECTOR,
              DT_POSINT_VECTOR,
              DT_FLOAT_VECTOR,
              DT_POSFLOAT_VECTOR,
              DT_FILE,
              DT_EXEC,
              DT_FOLDER,
              DT_DATE
              } dt_enum;

dt_enum conf_data_get_dt_from_string(
  const char * str);

bool conf_data_string_is_dt(
  const char * str);

const char * conf_data_get_dt_name_ref(
  dt_enum dt);

bool conf_data_validate_string_as_dt_value(
  dt_enum      dt,
  const char * str);

bool conf_data_validate_string_as_dt_vector(
  dt_enum      dt,
  const char * str,
  int        * num_elem);

int conf_data_get_int_from_string(
  dt_enum      dt,
  const char * str);

double conf_data_get_double_from_string(
  dt_enum      dt,
  const char * str);

time_t conf_data_get_time_t_from_string(
  dt_enum      dt,
  const char * str);

int_vector_type * conf_data_get_int_vector_from_string(
  dt_enum dt,
  const char * str);

double_vector_type * conf_data_get_double_vector_from_string(
  dt_enum dt,
  const char * str);

time_t_vector_type * conf_data_get_time_t_vector_from_string(
  dt_enum dt,
  const char * str);

#endif
