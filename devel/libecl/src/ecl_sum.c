/*
   Copyright (C) 2011  Statoil ASA, Norway.

   The file 'ecl_sum.c' is part of ERT - Ensemble based Reservoir Tool.

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

#include <string.h>
#include <stdbool.h>
#include <math.h>
#include <ecl_util.h>
#include <ecl_sum.h>
#include <hash.h>
#include <util.h>
#include <time.h>
#include <set.h>
#include <util.h>
#include <vector.h>
#include <int_vector.h>
#include <bool_vector.h>
#include <time_t_vector.h>
#include <ecl_smspec.h>
#include <ecl_sum_data.h>
#include <stringlist.h>

/**
   The ECLIPSE summary data is organised in a header file (.SMSPEC)
   and the actual summary data. This file implements a data structure
   ecl_sum_type which holds ECLIPSE summary data. Most of the actual
   implementation is in separate files ecl_smspec.c for the SMSPEC
   header, and ecl_sum_data for the actual data.

   Observe that this datastructure is built up around internalizing
   ECLIPSE summary data, the code has NO AMBITION of being able to
   write summary data.


   Header in ECLIPSE.SMSPEC file                  Actual data; many 'PARAMS' blocks in .Snnnn or .UNSMRY file

   ------------------------------.........       -----------   -----------   -----------   -----------   -----------
   | WGNAMES    KEYWORDS   NUMS | INDEX  :       | PARAMS  |   | PARAMS  |   | PARAMS  |   | PARAMS  |   | PARAMS  |
   |----------------------------|........:       |---------|   |---------|   |---------|   |---------|   |---------|
   | OP-1       WOPR       X    |   0    :       |  652    |   |   752   |   |  862    |   |   852   |   |    962  |
   | OP-1       WWPR       X    |   1    :       |   45    |   |    47   |   |   55    |   |    59   |   |     62  |
   | GI-1       WGIR       X    |   2    :       |  500    |   |   500   |   |  786    |   |   786   |   |    486  |
   | :+:+:+:+   FOPT       X    |   3    :       | 7666    |   |  7666   |   | 8811    |   |  7688   |   |   8649  |
   | :+:+:+:+   RPR        5    |   4    :       |  255    |   |   255   |   |  266    |   |   257   |   |    277  |
   | :+:+:+:+   BPR        3457 |   5    :       |  167    |   |   167   |   |  189    |   |   201   |   |    166  |
   ------------------------------.........       -----------   -----------   -----------   -----------   -----------

                                                 <------------------------ Time direction ------------------------->

   As illustrated in the figure above header information is stored in
   the SMSPEC file; the header information is organised in several
   keywords; at least the WGNAMES and KEYWORDS arrays, and often also
   the NUMS array. Together these three arrays uniquely specify a
   summary vector.

   The INDEX column in the header information is NOT part of the
   SMSPEC file, but an important part of the ecl_smspec
   implementation. The the values from WGNAMES/KEYWORDS/NUMS are
   combined to create a unique key; and the corresponding index is
   used to lookup a numerical value from the PARAMS vector with actual
   data.
   
   These matters are documented further in the ecl_smspec.c and
   ecl_sum_data.c files.
*/




#define ECL_SUM_ID          89067

/*****************************************************************/

struct ecl_sum_struct {
  UTIL_TYPE_ID_DECLARATION;
  ecl_smspec_type   * smspec;   /* Internalized version of the SMSPEC file. */
  ecl_sum_data_type * data;     /* The data - can be NULL. */
};



/**
   Reads the data from ECLIPSE summary files, can either be a list of
   files BASE.S0000, BASE.S0001, BASE.S0002,.. or one unified
   file. Formatted/unformatted is detected automagically.

   The actual loading is implemented in the ecl_sum_data.c file.
*/


static void ecl_sum_fread_realloc_data(ecl_sum_type * ecl_sum , const stringlist_type * data_files , bool include_restart) {
  if (ecl_sum->data != NULL)
    ecl_sum_free_data( ecl_sum );
  ecl_sum->data   = ecl_sum_data_fread_alloc( ecl_sum->smspec , data_files , include_restart);
}


static ecl_sum_type * ecl_sum_fread_alloc__(const char *header_file , const stringlist_type *data_files , const char * key_join_string, bool include_restart) {
  ecl_sum_type *ecl_sum = util_malloc( sizeof * ecl_sum , __func__);
  UTIL_TYPE_ID_INIT( ecl_sum , ECL_SUM_ID );
  ecl_sum->smspec = ecl_smspec_fread_alloc( header_file , key_join_string , include_restart);
  ecl_sum->data   = NULL;
  ecl_sum_fread_realloc_data(ecl_sum , data_files , include_restart);
  return ecl_sum;
}

/**
   This will explicitly load the summary specified by @header_file and
   @data_files, i.e. if the case has been restarted from another case,
   it will NOT look for old summary information - that functionality
   is only invoked when using ecl_sum_fread_alloc_case() function.
*/

ecl_sum_type * ecl_sum_fread_alloc(const char *header_file , const stringlist_type *data_files , const char * key_join_string) {
  return ecl_sum_fread_alloc__( header_file , data_files , key_join_string , false );
}



UTIL_SAFE_CAST_FUNCTION( ecl_sum , ECL_SUM_ID );
UTIL_IS_INSTANCE_FUNCTION( ecl_sum , ECL_SUM_ID );

/**
   This function frees the data from the ecl_sum instance and sets the
   data pointer to NULL. The SMSPEC data is still valid, and can be
   reused with calls to ecl_sum_fread_realloc_data().
*/

void ecl_sum_free_data( ecl_sum_type * ecl_sum ) {
  ecl_sum_data_free( ecl_sum->data );
  ecl_sum->data = NULL;
}


void ecl_sum_free( ecl_sum_type * ecl_sum ) {
  ecl_sum_free_data( ecl_sum );
  ecl_smspec_free( ecl_sum->smspec );
  free( ecl_sum );
}



void ecl_sum_free__(void * __ecl_sum) {
  ecl_sum_type * ecl_sum = ecl_sum_safe_cast( __ecl_sum);
  ecl_sum_free( ecl_sum );
}




/**
   This function takes an input file, and loads the corresponding
   summary. The function extracts the path part, and the basename from
   the input file. The extension is not considered (the input need not
   even be a valid file). In principle a simulation directory with a
   given basename can contain four different simulation cases:

    * Formatted and unformatted.
    * Unified and not unified.

   The program will load the most recent dataset, by looking at the
   modification time stamps of the files; if no simulation case is
   found the function will return NULL.

   If the SMSPEC file contains the RESTART keyword the function will
   iterate backwards to load summary information from previous runs
   (this is goverened by the local variable include_restart).
*/


ecl_sum_type * ecl_sum_fread_alloc_case(const char * input_file , const char * key_join_string){
  bool include_restart = true;
  return ecl_sum_fread_alloc_case__( input_file , key_join_string , include_restart );
}


ecl_sum_type * ecl_sum_fread_alloc_case__(const char * input_file , const char * key_join_string , bool include_restart){
  ecl_sum_type * ecl_sum     = NULL;
  char * path , * base, *ext;
  char * header_file;
  stringlist_type * summary_file_list = stringlist_alloc_new();

  util_alloc_file_components( input_file , &path , &base , &ext);
  
  
  /* Should add ext to the base if ext does not represent a valid ECLIPSE extension ?? */
  if (ecl_util_alloc_summary_files( path , base , ext , &header_file , summary_file_list ))
    ecl_sum = ecl_sum_fread_alloc__( header_file , summary_file_list , key_join_string , include_restart);

  util_safe_free( base );
  util_safe_free( path );
  util_safe_free( ext );
  util_safe_free( header_file );
  stringlist_free( summary_file_list );

  return ecl_sum;
}

/*****************************************************************/
/*
   Here comes lots of access functions - these are mostly thin
   wrapppers around ecl_smspec functions. See more 'extensive'
   documentation in ecl_smspec.c

   The functions returning an actual value,
   i.e. ecl_sum_get_well_var() will trustingly call ecl_sum_data_get()
   with whatever indices it gets. If the indices are invalid -
   ecl_sum_data_get() will abort. The abort is the 'correct'
   behaviour, but it is possible to abort in this scope as well, in
   that case more informative error message can be supplied (i.e. the
   well/variable B-33T2/WOPR does not exist, instead of just "invalid
   index" which is the best ecl_sum_data_get() can manage.).
*/

/*****************************************************************/
/* Well variables */

int     ecl_sum_get_well_var_index(const ecl_sum_type * ecl_sum , const char * well , const char *var) { return ecl_smspec_get_well_var_index(ecl_sum->smspec , well , var); }
bool    ecl_sum_has_well_var(const ecl_sum_type * ecl_sum , const char * well , const char *var)       { return ecl_smspec_has_well_var(ecl_sum->smspec , well , var); }

double  ecl_sum_get_well_var(const ecl_sum_type * ecl_sum , int time_index , const char * well , const char *var) {
  int index = ecl_sum_get_well_var_index( ecl_sum , well , var );
  return ecl_sum_data_iget( ecl_sum->data , time_index , index);
}

double ecl_sum_get_well_var_from_sim_time( const ecl_sum_type * ecl_sum , time_t sim_time , const char * well , const char * var) {
  int index = ecl_sum_get_well_var_index( ecl_sum , well , var );
  return ecl_sum_data_get_from_sim_time( ecl_sum->data , sim_time , index );
}

double ecl_sum_get_well_var_from_sim_days( const ecl_sum_type * ecl_sum , double sim_days , const char * well , const char * var) {
  int index = ecl_sum_get_well_var_index( ecl_sum , well , var );
  return ecl_sum_data_get_from_sim_days( ecl_sum->data , sim_days , index );
}


/*****************************************************************/
/* Group variables */

int  ecl_sum_get_group_var_index(const ecl_sum_type * ecl_sum , const char * group , const char *var) { return ecl_smspec_get_group_var_index( ecl_sum->smspec , group , var); }
bool ecl_sum_has_group_var(const ecl_sum_type * ecl_sum , const char * group , const char *var)       { return ecl_smspec_has_group_var( ecl_sum->smspec , group , var); }

double  ecl_sum_get_group_var(const ecl_sum_type * ecl_sum , int time_index , const char * group , const char *var) {
  int index = ecl_sum_get_group_var_index( ecl_sum , group , var );
  return ecl_sum_data_iget( ecl_sum->data , time_index , index);
}

double ecl_sum_get_group_var_from_sim_time( const ecl_sum_type * ecl_sum , time_t sim_time , const char * group , const char * var) {
  int index = ecl_sum_get_group_var_index( ecl_sum , group , var );
  return ecl_sum_data_get_from_sim_time( ecl_sum->data , sim_time , index );
}

double ecl_sum_get_group_var_from_sim_days( const ecl_sum_type * ecl_sum , double sim_days , const char * group , const char * var) {
  int index = ecl_sum_get_group_var_index( ecl_sum , group , var );
  return ecl_sum_data_get_from_sim_days( ecl_sum->data , sim_days , index );
}


/*****************************************************************/
/* Field variables */
int  ecl_sum_get_field_var_index(const ecl_sum_type * ecl_sum , const char *var) { return ecl_smspec_get_field_var_index( ecl_sum->smspec , var); }
bool ecl_sum_has_field_var(const ecl_sum_type * ecl_sum , const char *var)       { return ecl_smspec_has_field_var( ecl_sum->smspec , var); }

double ecl_sum_get_field_var(const ecl_sum_type * ecl_sum , int time_index , const char * var) {
  int index = ecl_sum_get_field_var_index( ecl_sum ,  var );
  return ecl_sum_data_iget( ecl_sum->data , time_index , index);
}

double ecl_sum_get_field_var_from_sim_time( const ecl_sum_type * ecl_sum , time_t sim_time , const char * var) {
  int index = ecl_sum_get_field_var_index( ecl_sum , var );
  return ecl_sum_data_get_from_sim_time( ecl_sum->data , sim_time , index );
}

double ecl_sum_get_field_var_from_sim_days( const ecl_sum_type * ecl_sum , double sim_days , const char * var) {
  int index = ecl_sum_get_field_var_index( ecl_sum , var );
  return ecl_sum_data_get_from_sim_days( ecl_sum->data , sim_days , index );
}


/*****************************************************************/
/* Block variables */

int  ecl_sum_get_block_var_index(const ecl_sum_type * ecl_sum , const char * block_var , int block_nr) { return ecl_smspec_get_block_var_index( ecl_sum->smspec , block_var , block_nr ); }
bool ecl_sum_has_block_var(const ecl_sum_type * ecl_sum , const char * block_var , int block_nr)       { return ecl_smspec_has_block_var( ecl_sum->smspec , block_var , block_nr ); }

double ecl_sum_get_block_var(const ecl_sum_type * ecl_sum , int time_index , const char * block_var , int block_nr) {
  int index = ecl_sum_get_block_var_index( ecl_sum ,  block_var , block_nr);
  return ecl_sum_data_iget( ecl_sum->data , time_index , index);
}


int  ecl_sum_get_block_var_index_ijk(const ecl_sum_type * ecl_sum , const char * block_var , int i, int j , int k ) {
  return ecl_smspec_get_block_var_index_ijk( ecl_sum->smspec , block_var , i , j , k);
}

bool ecl_sum_has_block_var_ijk(const ecl_sum_type * ecl_sum , const char * block_var , int i, int j , int k) {
  return ecl_smspec_has_block_var_ijk( ecl_sum->smspec , block_var , i ,j , k);
}

double ecl_sum_get_block_var_ijk(const ecl_sum_type * ecl_sum , int time_index , const char * block_var , int i , int j , int k) {
  int index = ecl_sum_get_block_var_index_ijk( ecl_sum ,  block_var , i , j , k);
  return ecl_sum_data_iget( ecl_sum->data , time_index , index);
}

double ecl_sum_get_block_var_ijk_from_sim_time( const ecl_sum_type * ecl_sum , time_t sim_time , const char * block_var, int i , int j , int k) {
  int index = ecl_sum_get_block_var_index_ijk( ecl_sum , block_var ,i,j,k);
  return ecl_sum_data_get_from_sim_time( ecl_sum->data , sim_time , index );
}

double ecl_sum_get_block_var_ijk_from_sim_days( const ecl_sum_type * ecl_sum , double sim_days , const char * block_var, int i , int j , int k) {
  int index = ecl_sum_get_block_var_index_ijk( ecl_sum , block_var ,i,j,k);
  return ecl_sum_data_get_from_sim_days( ecl_sum->data , sim_days , index );
}



/*****************************************************************/
/* Region variables */
/**
   region_nr: [1...num_regions] (NOT C-based indexing)
*/

int  ecl_sum_get_region_var_index(const ecl_sum_type * ecl_sum , int region_nr , const char *var) { return ecl_smspec_get_region_var_index( ecl_sum->smspec , region_nr , var); }
bool ecl_sum_has_region_var(const ecl_sum_type * ecl_sum , int region_nr , const char *var)       { return ecl_smspec_has_region_var( ecl_sum->smspec , region_nr , var); }

double ecl_sum_iget_region_var(const ecl_sum_type * ecl_sum , int time_index , int region_nr , const char *var) {
  int index = ecl_sum_get_region_var_index( ecl_sum ,  region_nr , var);
  return ecl_sum_data_iget( ecl_sum->data , time_index , index);
}

double ecl_sum_get_region_var_from_sim_time( const ecl_sum_type * ecl_sum , time_t sim_time , int region_nr , const char * var) {
  int index = ecl_sum_get_region_var_index( ecl_sum , region_nr , var );
  return ecl_sum_data_get_from_sim_time( ecl_sum->data , sim_time , index );
}

double ecl_sum_get_region_var_from_sim_days( const ecl_sum_type * ecl_sum , double sim_days , int region_nr , const char * var) {
  int index = ecl_sum_get_region_var_index( ecl_sum , region_nr , var );
  return ecl_sum_data_get_from_sim_days( ecl_sum->data , sim_days , index );
}



/*****************************************************************/
/* Misc variables */

int     ecl_sum_get_misc_var_index(const ecl_sum_type * ecl_sum , const char *var) { return ecl_smspec_get_misc_var_index( ecl_sum->smspec , var ); }
bool    ecl_sum_has_misc_var(const ecl_sum_type * ecl_sum , const char *var)       { return ecl_smspec_has_misc_var( ecl_sum->smspec , var ); }

double  ecl_sum_get_misc_var(const ecl_sum_type * ecl_sum , int time_index , const char *var) {
  int index = ecl_sum_get_misc_var_index( ecl_sum ,  var);
  return ecl_sum_data_iget( ecl_sum->data , time_index , index);
}

double ecl_sum_get_misc_var_from_sim_time( const ecl_sum_type * ecl_sum , time_t sim_time , const char * var) {
  int index = ecl_sum_get_misc_var_index( ecl_sum , var );
  return ecl_sum_data_get_from_sim_time( ecl_sum->data , sim_time , index );
}

double ecl_sum_get_misc_var_from_sim_days( const ecl_sum_type * ecl_sum , double sim_days , const char * var) {
  int index = ecl_sum_get_misc_var_index( ecl_sum , var );
  return ecl_sum_data_get_from_sim_days( ecl_sum->data , sim_days , index );
}



/*****************************************************************/
/* Well completion - not fully implemented ?? */

int ecl_sum_get_well_completion_var_index(const ecl_sum_type * ecl_sum , const char * well , const char *var, int cell_nr) {
  return ecl_smspec_get_well_completion_var_index( ecl_sum->smspec , well , var , cell_nr);
}

bool ecl_sum_has_well_completion_var(const ecl_sum_type * ecl_sum , const char * well , const char *var, int cell_nr)  {
  return ecl_smspec_has_well_completion_var( ecl_sum->smspec , well , var , cell_nr);
}

double ecl_sum_get_well_completion_var(const ecl_sum_type * ecl_sum , int time_index , const char * well , const char *var, int cell_nr)  {
  int index = ecl_sum_get_well_completion_var_index(ecl_sum , well , var , cell_nr);
  return ecl_sum_data_iget( ecl_sum->data , time_index , index);
}

/*****************************************************************/
/* General variables - this means WWCT:OP_1 - i.e. composite variables*/

int  ecl_sum_get_general_var_index(const ecl_sum_type * ecl_sum , const char * lookup_kw) {
  return ecl_smspec_get_general_var_index( ecl_sum->smspec , lookup_kw);
}

bool ecl_sum_has_general_var(const ecl_sum_type * ecl_sum , const char * lookup_kw)       { return ecl_smspec_has_general_var( ecl_sum->smspec , lookup_kw); }

double ecl_sum_get_general_var(const ecl_sum_type * ecl_sum , int time_index , const char * lookup_kw) {
  int index = ecl_sum_get_general_var_index(ecl_sum , lookup_kw);
  return ecl_sum_data_iget( ecl_sum->data , time_index , index);
}

double ecl_sum_iget_general_var(const ecl_sum_type * ecl_sum , int internal_index , const char * lookup_kw) {
  int index = ecl_sum_get_general_var_index(ecl_sum , lookup_kw);
  return ecl_sum_data_iget( ecl_sum->data , internal_index  , index);
}

double ecl_sum_get_general_var_from_sim_time( const ecl_sum_type * ecl_sum , time_t sim_time , const char * var) {
  int index = ecl_sum_get_general_var_index( ecl_sum , var );
  return ecl_sum_data_get_from_sim_time( ecl_sum->data , sim_time , index );
}

double ecl_sum_get_general_var_from_sim_days( const ecl_sum_type * ecl_sum , double sim_days , const char * var) {
  int index = ecl_sum_get_general_var_index( ecl_sum , var );
  return ecl_sum_data_get_from_sim_days( ecl_sum->data , sim_days , index );
}

const char * ecl_sum_get_general_var_unit( const ecl_sum_type * ecl_sum , const char * var) {
  return ecl_smspec_get_general_var_unit(ecl_sum->smspec , var );
}

/*****************************************************************/
/*
   Indexed get - these functions can be used after another function
   has been used to query for index.
*/



double ecl_sum_iiget( const ecl_sum_type * ecl_sum , int internal_index , int param_index) {
  return ecl_sum_data_iget(ecl_sum->data , internal_index , param_index);
}

const char * ecl_sum_iget_unit( const ecl_sum_type * ecl_sum , int param_index) {
  return ecl_smspec_iget_unit(ecl_sum->smspec , param_index);
}

int ecl_sum_iget_num( const ecl_sum_type * sum , int param_index ) {
  return ecl_smspec_iget_num( sum->smspec , param_index );
}

const char * ecl_sum_iget_wgname( const ecl_sum_type * sum , int param_index ) {
  return ecl_smspec_iget_wgname( sum->smspec , param_index );
}

const char * ecl_sum_iget_keyword( const ecl_sum_type * sum , int param_index ) {
  return ecl_smspec_iget_keyword( sum->smspec , param_index );
}

double ecl_sum_iget_from_sim_time( const ecl_sum_type * ecl_sum , time_t sim_time , int param_index) {
  return ecl_sum_data_get_from_sim_time( ecl_sum->data , sim_time , param_index );
}

double ecl_sum_iget_from_sim_days( const ecl_sum_type * ecl_sum , double sim_days , int param_index ) {
  return ecl_sum_data_get_from_sim_days( ecl_sum->data , sim_days , param_index );
}

/*****************************************************************/
/* Simple get functions which take a general var key as input    */

bool ecl_sum_var_is_rate( const ecl_sum_type * ecl_sum , const char * gen_key) {
  int index = ecl_sum_get_general_var_index( ecl_sum , gen_key );
  return ecl_smspec_is_rate( ecl_sum->smspec , index );
}

bool ecl_sum_var_is_total( const ecl_sum_type * ecl_sum , const char * gen_key) {
  return ecl_smspec_general_is_total( ecl_sum->smspec , gen_key );
}

/**
   The identify_var_type function does not consider the ecl_sum
   instance, only the characters in var. [Except if var is found in
   the special keys hash] - i.e. it is safe to call :

        ecl_sum_identify_var_type(ecl_sum , "WWCT:WELLXX")

   even though the current ecl_sum instance does not have a well named
   'WELLXX'.  
*/

ecl_smspec_var_type ecl_sum_identify_var_type(const ecl_sum_type * ecl_sum , const char * var) {
  return ecl_smspec_identify_var_type( ecl_sum->smspec , var );
}


ecl_smspec_var_type ecl_sum_get_var_type( const ecl_sum_type * ecl_sum , const char * gen_key) {
  int index = ecl_sum_get_general_var_index( ecl_sum , gen_key );
  return ecl_smspec_iget_var_type(ecl_sum->smspec , index);
}

const char * ecl_sum_get_unit( const ecl_sum_type * ecl_sum , const char * gen_key) {
  int index = ecl_sum_get_general_var_index( ecl_sum , gen_key );
  return ecl_smspec_iget_unit(ecl_sum->smspec , index);
}

int ecl_sum_get_num( const ecl_sum_type * sum , const char * gen_key ) {
  int index = ecl_sum_get_general_var_index( sum , gen_key );
  return ecl_smspec_iget_num( sum->smspec , index );
}

const char * ecl_sum_get_wgname( const ecl_sum_type * sum , const char * gen_key ) {
  int index = ecl_sum_get_general_var_index( sum , gen_key );
  return ecl_smspec_iget_wgname( sum->smspec , index );
}

const char * ecl_sum_get_keyword( const ecl_sum_type * sum , const char * gen_key ) {
  int index = ecl_sum_get_general_var_index( sum , gen_key );
  return ecl_smspec_iget_keyword( sum->smspec , index );
}



/*****************************************************************/
/*
   Here comes a couple of functions relating to the time
   dimension. The functions here in this file are just thin wrappers
   of 'real' functions located in ecl_sum_data.c.
*/



bool  ecl_sum_has_report_step(const ecl_sum_type * ecl_sum , int report_step ) {
  return ecl_sum_data_has_report_step( ecl_sum->data , report_step );
}

int ecl_sum_get_last_report_step( const ecl_sum_type * ecl_sum) {
  return ecl_sum_data_get_last_report_step( ecl_sum->data );
}

int ecl_sum_get_first_report_step( const ecl_sum_type * ecl_sum ) {
  return ecl_sum_data_get_first_report_step( ecl_sum->data );
}

int ecl_sum_iget_report_end( const ecl_sum_type * ecl_sum, int report_step) {
  return ecl_sum_data_iget_report_end(ecl_sum->data , report_step );
}

int ecl_sum_iget_report_start( const ecl_sum_type * ecl_sum, int report_step) {
  return ecl_sum_data_iget_report_start(ecl_sum->data , report_step );
}

int ecl_sum_iget_report_step( const ecl_sum_type * ecl_sum , int internal_index ){
  return ecl_sum_data_iget_report_step( ecl_sum->data , internal_index );
}


int ecl_sum_iget_mini_step( const ecl_sum_type * ecl_sum , int internal_index ){
  return ecl_sum_data_iget_mini_step( ecl_sum->data , internal_index );
}


void ecl_sum_init_time_vector( const ecl_sum_type * ecl_sum , time_t_vector_type * time_vector , bool report_only ) {
  ecl_sum_data_init_time_vector( ecl_sum->data , time_vector , report_only );
}


time_t_vector_type * ecl_sum_alloc_time_vector( const ecl_sum_type * ecl_sum  , bool report_only) {
  return ecl_sum_data_alloc_time_vector( ecl_sum->data , report_only );
}

void ecl_sum_init_data_vector( const ecl_sum_type * ecl_sum , double_vector_type * data_vector , int data_index , bool report_only ) {
  ecl_sum_data_init_data_vector( ecl_sum->data , data_vector , data_index , report_only );
}


double_vector_type * ecl_sum_alloc_data_vector( const ecl_sum_type * ecl_sum  , int data_index , bool report_only) {
  return ecl_sum_data_alloc_data_vector( ecl_sum->data , data_index , report_only );
}



void ecl_sum_summarize( const ecl_sum_type * ecl_sum , FILE * stream ) {
  ecl_sum_data_summarize( ecl_sum->data , stream );
}



/**
   Returns the first internal index where a limiting value is
   reached. If the limiting value is never reached, -1 is
   returned. The smspec_index should be calculated first with one of
   the

      ecl_sum_get_XXXX_index()

   functions. I.e. the following code will give the first index where
   the water wut in well PX exceeds 0.25:

   {
      int smspec_index   = ecl_sum_get_well_var( ecl_sum , "PX" , "WWCT" );
      int first_index    = ecl_sum_get_first_gt( ecl_sum , smspec_index , 0.25);
   }

*/


static int ecl_sum_get_limiting(const ecl_sum_type * ecl_sum , int smspec_index , double limit , bool gt) {
  const int length        = ecl_sum_data_get_length( ecl_sum->data );
  int internal_index      = 0;
  do {
    double value = ecl_sum_data_iget( ecl_sum->data , internal_index , smspec_index );
    if (gt) {
      if (value > limit)
        break;
    } else {
      if (value < limit)
        break;
    }
    internal_index++;
  } while (internal_index < length);

  if (internal_index == length)  /* Did not find it */
    internal_index = -1;

  return internal_index;
}

int ecl_sum_get_first_gt( const ecl_sum_type * ecl_sum , int param_index , double limit) {
  return ecl_sum_get_limiting( ecl_sum , param_index , limit , true );
}

int ecl_sum_get_first_lt( const ecl_sum_type * ecl_sum , int param_index , double limit) {
  return ecl_sum_get_limiting( ecl_sum , param_index , limit , false );
}



time_t ecl_sum_get_report_time( const ecl_sum_type * ecl_sum , int report_step ) {
  int end_index = ecl_sum_data_iget_report_end( ecl_sum->data , report_step );
  return ecl_sum_iget_sim_time( ecl_sum , end_index );
}

time_t ecl_sum_iget_sim_time( const ecl_sum_type * ecl_sum , int index ) {
  return ecl_sum_data_iget_sim_time( ecl_sum->data , index );
}

time_t ecl_sum_get_data_start( const ecl_sum_type * ecl_sum ) {
  return ecl_sum_data_get_data_start( ecl_sum->data );
}

time_t ecl_sum_get_start_time( const ecl_sum_type * ecl_sum) {
  return ecl_smspec_get_start_time( ecl_sum->smspec );
}

time_t ecl_sum_get_end_time( const ecl_sum_type * ecl_sum) {
  return ecl_sum_data_get_sim_end( ecl_sum->data );
}

double ecl_sum_iget_sim_days( const ecl_sum_type * ecl_sum , int index ) {
  return ecl_sum_data_iget_sim_days( ecl_sum->data , index );
}


/*****************************************************************/
/* This is essentially the summary.x program. */

#define DAYS_DATE_FORMAT    "%7.2f   %02d/%02d/%04d   "
#define FLOAT_FORMAT        " %12.3f "
#define HEADER_FORMAT       " %12s "
#define DATE_HEADER         "-- Days   dd/mm/yyyy   "
#define DATE_DASH           "-----------------------"
#define FLOAT_DASH          "--------------"

static void __ecl_sum_fprintf_line( const ecl_sum_type * ecl_sum , FILE * stream , int internal_index , const bool_vector_type * has_var , const int_vector_type * var_index) {
  int ivar , day,month,year;
  util_set_date_values(ecl_sum_iget_sim_time(ecl_sum , internal_index ) , &day , &month, &year);
  fprintf(stream , DAYS_DATE_FORMAT , ecl_sum_iget_sim_days(ecl_sum , internal_index) , day , month , year);

  for (ivar = 0; ivar < int_vector_size( var_index ); ivar++)
    if (bool_vector_iget( has_var , ivar ))
      fprintf(stream , FLOAT_FORMAT , ecl_sum_iiget(ecl_sum , internal_index, int_vector_iget( var_index , ivar )));

  fprintf(stream , "\n");
}


static void ecl_sum_fprintf_header( const ecl_sum_type * ecl_sum , const stringlist_type * key_list , const bool_vector_type * has_var , FILE * stream) {
  fprintf(stream , DATE_HEADER);
  for (int i=0; i < stringlist_get_size( key_list ); i++)
    if (bool_vector_iget( has_var , i ))
      fprintf(stream , HEADER_FORMAT , stringlist_iget( key_list , i ));
  fprintf( stream , "\n");

  fprintf(stream , DATE_DASH);
  for (int i=0; i < stringlist_get_size( key_list ); i++)
    if (bool_vector_iget( has_var , i ))
      fprintf(stream , FLOAT_DASH);
  fprintf( stream , "\n");
}


void ecl_sum_fprintf(const ecl_sum_type * ecl_sum , FILE * stream , const stringlist_type * var_list , bool report_only , bool print_header) {
  bool_vector_type  * has_var   = bool_vector_alloc( stringlist_get_size( var_list ), false );
  int_vector_type   * var_index = int_vector_alloc( stringlist_get_size( var_list ), -1 );

  {
    int ivar;
    for (ivar = 0; ivar < stringlist_get_size( var_list ); ivar++) {
      if (ecl_sum_has_general_var( ecl_sum , stringlist_iget( var_list , ivar) )) {
        bool_vector_iset( has_var , ivar , true );
        int_vector_iset( var_index , ivar , ecl_sum_get_general_var_index( ecl_sum , stringlist_iget( var_list , ivar) ));
      } else {
        fprintf(stderr,"** Warning: could not find variable: \'%s\' in summary file \n", stringlist_iget( var_list , ivar));
        bool_vector_iset( has_var , ivar , false);
      }
    }
  }

  if (print_header)
    ecl_sum_fprintf_header( ecl_sum , var_list , has_var , stream );

  if (report_only) {
    int first_report = ecl_sum_get_first_report_step( ecl_sum );
    int last_report  = ecl_sum_get_last_report_step( ecl_sum );
    int report;

    for (report = first_report; report <= last_report; report++) {
      if (ecl_sum_data_has_report_step(ecl_sum->data , report)) {
        int internal_index;
        internal_index = ecl_sum_data_iget_report_end( ecl_sum->data , report );
        __ecl_sum_fprintf_line( ecl_sum , stream , internal_index , has_var , var_index );
      }
    }
  } else {
    int internal_index;
    for (internal_index = 0; internal_index < ecl_sum_get_data_length( ecl_sum ); internal_index++)
      __ecl_sum_fprintf_line( ecl_sum , stream , internal_index , has_var , var_index );
  }

  int_vector_free( var_index );
  bool_vector_free( has_var );
}


const char * ecl_sum_get_case(const ecl_sum_type * ecl_sum) {
  return ecl_smspec_get_simulation_case( ecl_sum->smspec );
}


/**
   This function will check if the currently loaded case corresponds
   to the case specified by @input_file. The extension of @input file
   can be arbitrary (or nonexistent) and will be ignored (this can
   lead to errors with formatted/unformatted mixup if the simulation
   directory has been changed after the ecl_sum instance has been
   loaded).
*/


bool ecl_sum_same_case( const ecl_sum_type * ecl_sum , const char * input_file ) {
  bool   same_case = false;
  {
    char * path;
    char * base;

    util_alloc_file_components( input_file , &path , &base , NULL);
    {
      bool   fmt_file = ecl_smspec_get_formatted( ecl_sum->smspec );
      char * header_file = ecl_util_alloc_exfilename( path , base , ECL_SUMMARY_HEADER_FILE , fmt_file , -1 );
      if (header_file != NULL) {
        same_case = util_same_file( header_file , ecl_smspec_get_header_file( ecl_sum->smspec ));
        free( header_file );
      }
    }

    util_safe_free( path );
    util_safe_free( base );
  }
  return same_case;
}




/*****************************************************************/


stringlist_type * ecl_sum_alloc_matching_general_var_list(const ecl_sum_type * ecl_sum , const char * pattern) {
  return ecl_smspec_alloc_matching_general_var_list(ecl_sum->smspec , pattern );
}

void ecl_sum_select_matching_general_var_list( const ecl_sum_type * ecl_sum , const char * pattern , stringlist_type * keys) {
  ecl_smspec_select_matching_general_var_list( ecl_sum->smspec , pattern , keys );
}

stringlist_type * ecl_sum_alloc_well_list( const ecl_sum_type * ecl_sum , const char * pattern) {
  return ecl_smspec_alloc_well_list( ecl_sum->smspec , pattern );
}

stringlist_type * ecl_sum_alloc_well_var_list( const ecl_sum_type * ecl_sum ) {
  return ecl_smspec_alloc_well_var_list( ecl_sum->smspec );
}



/*****************************************************************/


void ecl_sum_resample_from_sim_time( const ecl_sum_type * ecl_sum , const time_t_vector_type * sim_time , double_vector_type * value , const char * gen_key) {
  int param_index = ecl_smspec_get_general_var_index( ecl_sum->smspec , gen_key );
  double_vector_reset( value );
  {
    int i;
    for (i=0; i < time_t_vector_size( sim_time ); i++)
      double_vector_iset( value , i , ecl_sum_data_get_from_sim_time( ecl_sum->data , time_t_vector_iget( sim_time , i ) , param_index));
  }
}


void ecl_sum_resample_from_sim_days( const ecl_sum_type * ecl_sum , const double_vector_type * sim_days , double_vector_type * value , const char * gen_key) {
  int param_index = ecl_smspec_get_general_var_index( ecl_sum->smspec , gen_key );
  double_vector_reset( value );
  {
    int i;
    for (i=0; i < double_vector_size( sim_days ); i++)
      double_vector_iset( value , i , ecl_sum_data_get_from_sim_days( ecl_sum->data , double_vector_iget( sim_days , i ) , param_index));
  }
}


time_t ecl_sum_time_from_days( const ecl_sum_type * ecl_sum , double sim_days ) {
  time_t t = ecl_smspec_get_start_time( ecl_sum->smspec );
  util_inplace_forward_days( &t , sim_days );
  return t;
}


double ecl_sum_days_from_time( const ecl_sum_type * ecl_sum , time_t sim_time ) {
  double seconds_diff = util_difftime( ecl_smspec_get_start_time( ecl_sum->smspec ) , sim_time , NULL , NULL , NULL, NULL);
  return seconds_diff * 1.0 / (3600 * 24.0);
}


double ecl_sum_get_first_day( const ecl_sum_type * ecl_sum ) {
  return ecl_sum_data_get_first_day( ecl_sum->data );
}

double ecl_sum_get_sim_length( const ecl_sum_type * ecl_sum ) {
  return ecl_sum_data_get_sim_length( ecl_sum->data );
}

/**
   Will return the number of data blocks.
*/
int ecl_sum_get_data_length( const ecl_sum_type * ecl_sum ) {
  return ecl_sum_data_get_length( ecl_sum->data );
}


bool ecl_sum_check_sim_time( const ecl_sum_type * sum , time_t sim_time) {
  return ecl_sum_data_check_sim_time( sum->data , sim_time );
}


bool ecl_sum_check_sim_days( const ecl_sum_type * sum , double sim_days) {
  return ecl_sum_data_check_sim_days( sum->data , sim_days );
}



/*****************************************************************/

const ecl_smspec_type * ecl_sum_get_smspec( const ecl_sum_type * ecl_sum ) {
  return ecl_sum->smspec;
}
