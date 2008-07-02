#ifndef __SCHED_KW_WELSPECS_H__
#define __SCHED_KW_WELSPECS_H__
#include <stdio.h>

/*
  Define the maximum number of keywords in a WELSPEC record.
  Note that this includes FrontSim and ECLIPSE 300 KWs.
*/
#define WELSPEC_NUM_KW 16
#define ECL_DEFAULT_KW "*"

/*************************************************************/

typedef struct sched_kw_welspecs_struct sched_kw_welspecs_type;

sched_kw_welspecs_type * sched_kw_welspecs_alloc();
sched_kw_welspecs_type * sched_kw_welspecs_fread_alloc(FILE *);

void sched_kw_welspecs_add_line(sched_kw_welspecs_type *, const char *);
void sched_kw_welspecs_free(sched_kw_welspecs_type *);
void sched_kw_welspecs_fprintf(const sched_kw_welspecs_type *, FILE *);
void sched_kw_welspecs_fwrite(const sched_kw_welspecs_type *, FILE *);


#endif
