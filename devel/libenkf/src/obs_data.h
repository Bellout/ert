#ifndef __OBS_DATA_H__
#define __OBS_DATA_H__
#include <stdio.h>

typedef struct obs_data_struct obs_data_type;

obs_data_type * obs_data_alloc();
void            obs_data_free(obs_data_type *);
void            obs_data_add(obs_data_type * , double , double , const char * );
void            obs_data_reset(obs_data_type * obs_data);
void            obs_data_fprintf(const obs_data_type * , FILE *);
double        * obs_data_allocD(const obs_data_type * , int , int , int , const double * , bool , double ** );
double        * obs_data_allocR(obs_data_type * , int , int , int , const double * , const double * , double );
double        * obs_data_alloc_innov(const obs_data_type * , int  , int , int ,  const double *);
void            obs_data_scale(const obs_data_type * , int , int , int , double * , double *, double *, double * , double *);
int             obs_data_get_nrobs(const obs_data_type * );

#endif
