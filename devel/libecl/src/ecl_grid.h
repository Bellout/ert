/*
   Copyright (C) 2011  Statoil ASA, Norway. 
    
   The file 'ecl_grid.h' is part of ERT - Ensemble based Reservoir Tool. 
    
   ERT is free software: you can redistribute it and/or modify 
   it under the terms of the GNU General Public License as published by 
   the Free Software Foundation, either version 3 of the License, or 
   (at your option) any later version. 
    
   ERT is distributed in the hope that it will be useful, but WITHOUT ANY 
   WARRANTY; without even the implied warranty of MERCHANTABILITY or 
   FITNESS FOR A PARTICULAR PURPOSE.   
    
   See the GNU General Public License at <http://www.gnu.org/licenses/gpl.html> 
   for more details. 
*/

#ifndef __ECL_GRID_H__
#define __ECL_GRID_H__
#ifdef __cplusplus
extern "C" {
#endif
#include <stdbool.h>
#include <double_vector.h>
#include <int_vector.h>
#include <ecl_kw.h>
#include <stringlist.h>

//typedef enum {
//  NO_HINT          = 0,
//  NORTH            = 1,
//  CENTRAL          = 2,
//  SOUTH            = 4,
//  
  


typedef double (block_function_ftype) ( const double_vector_type *); 
typedef struct ecl_grid_struct ecl_grid_type;

void            ecl_grid_get_column_property(const ecl_grid_type * ecl_grid , const ecl_kw_type * ecl_kw , int i , int j, double_vector_type * column);
int             ecl_grid_get_global_index_from_xy_top( const ecl_grid_type * ecl_grid , double x , double y);
int             ecl_grid_get_global_index_from_xy_bottom( const ecl_grid_type * ecl_grid , double x , double y);

void            ecl_grid_get_corner_xyz3(const ecl_grid_type * grid , int i , int j , int k, int corner_nr , double * xpos , double * ypos , double * zpos );
void            ecl_grid_get_corner_xyz1(const ecl_grid_type * grid , int global_index , int corner_nr , double * xpos , double * ypos , double * zpos );

double          ecl_grid_get_cell_thickness3( const ecl_grid_type * grid , int i , int j , int k);
double          ecl_grid_get_cell_thickness1( const ecl_grid_type * grid , int global_index );
double          ecl_grid_get_cdepth1(const ecl_grid_type * grid , int global_index);
double          ecl_grid_get_cdepth3(const ecl_grid_type * grid , int i, int j , int k);
double          ecl_grid_get_depth3(const ecl_grid_type * grid , int i, int j , int k);
int             ecl_grid_get_global_index_from_xy( const ecl_grid_type * ecl_grid , int k , bool lower_layer , double x , double y);
bool            ecl_grid_cell_contains_xyz1( const ecl_grid_type * ecl_grid , int global_index , double x , double y , double z);
bool            ecl_grid_cell_contains_xyz3( const ecl_grid_type * ecl_grid , int i , int j , int k, double x , double y , double z );
double          ecl_grid_get_cell_volume1( const ecl_grid_type * ecl_grid, int global_index );
double          ecl_grid_get_cell_volume3( const ecl_grid_type * ecl_grid, int i , int j , int k);
bool            ecl_grid_cell_contains1(const ecl_grid_type * grid , int global_index , double x , double y , double z);
bool            ecl_grid_cell_contains3(const ecl_grid_type * grid , int i , int j ,int k , double x , double y , double z);
int             ecl_grid_get_global_index_from_xyz(ecl_grid_type * grid , double x , double y , double z , int start_index);
const  char   * ecl_grid_get_name( const ecl_grid_type * );
int             ecl_grid_get_active_index3(const ecl_grid_type * ecl_grid , int i , int j , int k);
int             ecl_grid_get_active_index1(const ecl_grid_type * ecl_grid , int global_index);
bool            ecl_grid_cell_active3(const ecl_grid_type * , int  , int  , int );
bool            ecl_grid_cell_active1(const ecl_grid_type * , int);
inline bool     ecl_grid_ijk_valid(const ecl_grid_type * , int  , int , int ); 
inline int      ecl_grid_get_global_index3(const ecl_grid_type * , int  , int , int );
int             ecl_grid_get_global_index1A(const ecl_grid_type * ecl_grid , int active_index);
ecl_grid_type * ecl_grid_alloc_GRDECL_kw( const ecl_kw_type * gridhead_kw , const ecl_kw_type * zcorn_kw , const ecl_kw_type * coord_kw , const ecl_kw_type * actnum_kw , const ecl_kw_type * mapaxes_kw );
ecl_grid_type * ecl_grid_alloc_GRDECL_data(int , int , int , const float *  , const float *  , const int * , const float * mapaxes);
ecl_grid_type * ecl_grid_alloc(const char * );
ecl_grid_type * ecl_grid_load_case( const char * case_input );
bool            ecl_grid_exists( const char * case_input );
char          * ecl_grid_alloc_case_filename( const char * case_input );

void            ecl_grid_free(ecl_grid_type * );
void            ecl_grid_free__( void * arg );
void            ecl_grid_get_dims(const ecl_grid_type * , int *, int * , int * , int *);
int             ecl_grid_get_nz( const ecl_grid_type * grid );
int             ecl_grid_get_nx( const ecl_grid_type * grid );
int             ecl_grid_get_ny( const ecl_grid_type * grid );
int             ecl_grid_get_active_index(const ecl_grid_type *  , int  , int  , int );
void            ecl_grid_summarize(const ecl_grid_type * );
void            ecl_grid_get_ijk1(const ecl_grid_type * , int global_index , int *, int * , int *);
void            ecl_grid_get_ijk1A(const ecl_grid_type * , int active_index, int *, int * , int *);
void            ecl_grid_get_ijk_from_active_index(const ecl_grid_type *, int , int *, int * , int * );
void            ecl_grid_get_xyz3(const ecl_grid_type * , int , int , int , double * , double * , double *);
void            ecl_grid_get_xyz1(const ecl_grid_type * grid , int global_index , double *xpos , double *ypos , double *zpos);
void            ecl_grid_get_xyz1A(const ecl_grid_type * grid , int active_index , double *xpos , double *ypos , double *zpos);
int             ecl_grid_get_global_size( const ecl_grid_type * ecl_grid );
bool            ecl_grid_compare(const ecl_grid_type * g1 , const ecl_grid_type * g2);
int             ecl_grid_get_active_size( const ecl_grid_type * ecl_grid );

double          ecl_grid_get_bottom1(const ecl_grid_type * grid , int global_index);
double          ecl_grid_get_bottom3(const ecl_grid_type * grid , int i, int j , int k);
double          ecl_grid_get_bottom1A(const ecl_grid_type * grid , int active_index);
double          ecl_grid_get_top1(const ecl_grid_type * grid , int global_index);
double          ecl_grid_get_top3(const ecl_grid_type * grid , int i, int j , int k);
double          ecl_grid_get_top1A(const ecl_grid_type * grid , int active_index);
double          ecl_grid_get_top2(const ecl_grid_type * grid , int i, int j);
double          ecl_grid_get_bottom2(const ecl_grid_type * grid , int i, int j);
int             ecl_grid_locate_depth( const ecl_grid_type * grid , double depth , int i , int j );

void            ecl_grid_alloc_blocking_variables(ecl_grid_type * , int );
void            ecl_grid_init_blocking(ecl_grid_type * );
double          ecl_grid_block_eval2d(ecl_grid_type * grid , int i, int j , block_function_ftype * blockf );
double          ecl_grid_block_eval3d(ecl_grid_type * grid , int i, int j , int k ,block_function_ftype * blockf );
int             ecl_grid_get_block_count3d(const ecl_grid_type * ecl_grid , int i , int j, int k);
int             ecl_grid_get_block_count2d(const ecl_grid_type * ecl_grid , int i , int j);
bool            ecl_grid_block_value_2d(ecl_grid_type * , double  , double  ,double );
bool            ecl_grid_block_value_3d(ecl_grid_type * , double  , double  ,double , double);



/* lgr related functions */
const ecl_grid_type   * ecl_grid_get_cell_lgr3(const ecl_grid_type * grid , int i, int j , int k);
const ecl_grid_type   * ecl_grid_get_cell_lgr1A(const ecl_grid_type * grid , int active_index);
const ecl_grid_type   * ecl_grid_get_cell_lgr1(const ecl_grid_type * grid , int global_index );
int                     ecl_grid_get_num_lgr(const ecl_grid_type * main_grid );
int                     ecl_grid_get_grid_nr( const ecl_grid_type * ecl_grid );
ecl_grid_type         * ecl_grid_iget_lgr(const ecl_grid_type * main_grid , int lgr_nr);
ecl_grid_type         * ecl_grid_get_lgr(const ecl_grid_type * main_grid, const char * __lgr_name);
bool                    ecl_grid_has_lgr(const ecl_grid_type * main_grid, const char * __lgr_name);
stringlist_type       * ecl_grid_alloc_lgr_name_list(const ecl_grid_type * ecl_grid);
int                     ecl_grid_get_parent_cell1( const ecl_grid_type * grid , int global_index);
int                     ecl_grid_get_parent_cell3( const ecl_grid_type * grid , int i , int j , int k);
const ecl_grid_type   * ecl_grid_get_global_grid( const ecl_grid_type * grid );
bool                    ecl_grid_is_lgr( const ecl_grid_type * ecl_grid );
double                  ecl_grid_get_property(const ecl_grid_type * ecl_grid , const ecl_kw_type * ecl_kw , int i , int j , int k);
void                    ecl_grid_grdecl_fprintf_kw( const ecl_grid_type * ecl_grid , const ecl_kw_type * ecl_kw , FILE * stream , double double_default);

#ifdef __cplusplus
}
#endif
#endif
