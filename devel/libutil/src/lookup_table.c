#include <double_vector.h>
#include <stdbool.h>
#include <util.h>
#include <lookup_table.h>

struct lookup_table_struct {
  bool                 data_owner; 
  double_vector_type * x_vector;
  double_vector_type * y_vector;
  const double       * x_data;
  const double       * y_data; 
  int                  size; 
  double               xmin;
  double               xmax;
  bool                 prev_index;
};



void lookup_table_sort_data( lookup_table_type * lt) {
  if (double_vector_get_read_only( lt->x_vector ))
    if (!double_vector_is_sorted( lt->x_vector , false))
      util_abort("%s: x vector is not sorted and read-only - this will not fly\n",__func__);
  
  {
    int * sort_perm = double_vector_alloc_sort_perm( lt->x_vector );
    double_vector_permute( lt->x_vector , sort_perm );
    double_vector_permute( lt->y_vector , sort_perm );
    free( sort_perm );
  }
  
  lt->xmin = double_vector_get_first( lt->x_vector );
  lt->xmax = double_vector_get_last( lt->x_vector );
  lt->size = double_vector_size( lt->x_vector);
  lt->prev_index = -1;
  lt->x_data = double_vector_get_const_ptr( lt->x_vector );
  lt->y_data = double_vector_get_const_ptr( lt->y_vector );
}


/**
   IFF the @read_only flag is set to true; the x vector MUST be
   sorted.  
*/

void lookup_table_set_data( lookup_table_type * lt , double_vector_type * x , double_vector_type * y , bool data_owner ) {
  
  if (lt->data_owner) {
    double_vector_free( lt->x_vector );
    double_vector_free( lt->y_vector );
  }

  lt->x_vector = x;
  lt->y_vector = y;
  lt->data_owner = data_owner;
  lookup_table_sort_data( lt );
}



lookup_table_type * lookup_table_alloc( double_vector_type * x , double_vector_type * y , bool data_owner) {
  lookup_table_type * lt = util_malloc( sizeof * lt , __func__ );
  lt->data_owner = false;
  if ((x == NULL) && (y == NULL)) {
    x = double_vector_alloc(0 , 0);
    y = double_vector_alloc(0 , 0);
    data_owner = true;
  } 
  lookup_table_set_data( lt , x , y , false );
  lt->data_owner = data_owner;
  
  return lt;
}


void lookup_table_append( lookup_table_type * lt , double x , double y) {
  double_vector_append( lt->x_vector , x );
  double_vector_append( lt->y_vector , y );
}


void lookup_table_free( lookup_table_type * lt ) {
  if (lt->data_owner) {
    double_vector_free( lt->x_vector );
    double_vector_free( lt->y_vector );
  } 
  free( lt );
}




double lookup_table_interp( lookup_table_type * lt , double x) {
  if ((x >= lt->xmin) && (x < lt->xmax)) {
    int index = double_vector_lookup_bin__( lt->x_vector , x , lt->prev_index );
    printf("%g -> %d \n",x,index);
    {
      double x1 = lt->x_data[ index ];
      double x2 = lt->x_data[ index + 1];
      double y1 = lt->y_data[ index ];
      double y2 = lt->y_data[ index + 1];
      
      lt->prev_index = index;
      return (( x - x1 ) * y2 + (x2 - x) * y1) / (x2 - x1 );
    }
  } else {
    util_abort("%s: out of bounds \n",__func__);
    return -1;
  }
}


