#include <stdlib.h>
#include <util.h>
#include <string.h>
#include <math.h>
#include <plot_driver.h>
#include <plot_const.h>
#include <arg_pack.h>


typedef struct {
  char       * plot_path;
} text_state_type;





static text_state_type * text_state_alloc( void * init_arg ) {
  text_state_type * state = util_malloc( sizeof * state , __func__);
  {
    arg_pack_type * arg_pack = arg_pack_safe_cast( init_arg );
    state->plot_path         = util_alloc_string_copy( arg_pack_iget_ptr( arg_pack , 0) );
    util_make_path( state->plot_path );
  }
  
  return state;
}


static void text_state_close( text_state_type * state ) {
  free( state->plot_path );
  free( state );
}



/*****************************************************************/





static void text_close_driver( plot_driver_type * driver ) {
  text_state_close( driver->state );
}


char * text_alloc_filename( const char * plot_path , const char * label) {
  return util_alloc_filename(plot_path , label , NULL);
}


FILE * text_fopen(const plot_driver_type * driver , const char * label) {
  text_state_type * state = driver->state;
  char * filename = text_alloc_filename( state->plot_path , label );
  FILE * stream   = util_fopen( filename , "w");
  free( filename );
  return stream;
}


static void text_fprintf1(plot_driver_type * driver , const char * label , const double_vector_type * d) {
  FILE * stream = text_fopen( driver , label );

  for (int i=0; i < double_vector_size( d ); i++) 
    fprintf(stream , "%12.7f  \n",double_vector_iget(d , i));
  
  fclose( stream );
}



static void text_fprintf2(plot_driver_type * driver , const char * label , const double_vector_type * d1 , const double_vector_type * d2) {
  FILE * stream = text_fopen( driver , label );

  for (int i=0; i < double_vector_size( d1 ); i++) 
    fprintf(stream , "%12.7f  %12.7f  \n",double_vector_iget(d1 , i) , double_vector_iget(d2 , i));
  
  fclose( stream );
}


static void text_fprintf3(plot_driver_type * driver , const char * label , const double_vector_type * d1 , const double_vector_type * d2, const double_vector_type * d3) {
  FILE * stream = text_fopen( driver , label );

  for (int i=0; i < double_vector_size( d1 ); i++) 
    fprintf(stream , "%12.7f  %12.7f  %12.7f  \n",double_vector_iget(d1 , i) , double_vector_iget(d2 , i), double_vector_iget(d3 , i));
  
  fclose( stream );
}





void text_plot_xy1y2(plot_driver_type * driver     , 
                       const char * label , 
                       const double_vector_type * x  , 
                       const double_vector_type * y1  , 
                       const double_vector_type * y2  , 
                       line_attribute_type line_attr) {
  
  text_fprintf3( driver , label , x , y1 , y2);
  
}





void text_plot_x1x2y(plot_driver_type * driver      , 
                       const char * label             , 
                       const double_vector_type * x1  , 
                       const double_vector_type * x2  , 
                       const double_vector_type * y   , 
                       line_attribute_type line_attr) {

  text_fprintf3( driver , label , x1 , x2 , y);

}






void text_plot_xy(plot_driver_type * driver     , 
                    const char * label , 
                    const double_vector_type * x  , 
                    const double_vector_type * y  , 
                    plot_style_type style         , 
                    line_attribute_type line_attr , 
                    point_attribute_type point_attr) {

  text_fprintf2( driver , label , x ,  y);
  
}




void text_plot_hist( plot_driver_type * driver, const char * label , const double_vector_type * x , line_attribute_type line_attr) {

  text_fprintf1( driver , label , x );

}


/**
   init_arg should be an arg_pack instance with one string; the string
   should be the name of a directory, wherein all the plot files will
   be put. If the directory does not exist, it will be created.

   Example from calling scope:
   ---------------------------
   {
      arg_pack_type * arg_pack = arg_pack_alloc();
      arg_pack_append_ptr( arg_pack , plot_path );

      plot = plot_alloc( "TEXT" , arg_pack );
      arg_pack_free( arg_pack );
   }
*/
   

plot_driver_type * text_driver_alloc(void * init_arg) {
  plot_driver_type * driver = plot_driver_alloc_empty(PLPLOT , "TEXT");
  driver->state           = text_state_alloc( init_arg );
  
  driver->close_driver 	  = text_close_driver;
  driver->plot_xy         = text_plot_xy;
  driver->plot_xy1y2      = text_plot_xy1y2;
  driver->plot_x1x2y      = text_plot_x1x2y;
  driver->plot_hist       = text_plot_hist;
  return driver;
}
