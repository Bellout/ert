#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <ecl_kw.h>
#include <fortio.h>
#include <string.h>
#include <util.h>
#include <ecl_util.h>



void file_convert(const char * src_file , const char * target_file, ecl_file_enum file_type , bool fmt_src) {
  fortio_type *src , *target;
  ecl_kw_type * ecl_kw;
  bool formatted_src;

  printf("Converting %s -> %s \n",src_file , target_file);
  if (file_type != ecl_other_file)
    formatted_src = fmt_src;
  else {
    if (util_fmt_bit8(src_file)) 
      formatted_src = true;
    else
      formatted_src = false;
  }
  
  target = fortio_fopen(target_file , "w" , true , !formatted_src);
  src    = fortio_fopen(src_file , "r" , true , formatted_src);
  ecl_kw = ecl_kw_fread_alloc(src);
  if (ecl_kw == NULL) {
    fprintf(stderr,"Loading: %s failed - maybe you forgot the header? \n", src_file);
    abort();
  }

  while (ecl_kw != NULL) {
    ecl_kw_fwrite(ecl_kw , target);
    
    ecl_kw_free(ecl_kw);
    ecl_kw = ecl_kw_fread_alloc(src);
  }
  if (ecl_kw != NULL) ecl_kw_free(ecl_kw);
  fortio_fclose(src);
  fortio_fclose(target);
}


int main (int argc , char **argv) { 
  if (argc == 1) {
    fprintf(stderr,"Usage: convert.x <filename1> <filename2> <filename3> ...\n");
    exit(1);
  } else {

    char *src_file    = argv[1];
    char *target_file;
  
    int           report_nr;
    ecl_file_enum file_type;
    bool          fmt_file;
    ecl_util_get_file_type(src_file , &file_type , &fmt_file , &report_nr);
    
    if (file_type == ecl_other_file) {
      if (argc != 3) {
	fprintf(stderr,"When the file can not be recognized on the name as an ECLIPSE file you must give output_file as second (and final) argument \n");
	exit(0);
      }
      target_file = argv[2];
      file_convert(src_file , target_file , file_type , fmt_file);
    } else {
      int file_nr;
      for (file_nr = 1; file_nr < argc; file_nr++) {
	char *path;
	char *basename;
	char *extension;
	src_file    = argv[file_nr];
	ecl_util_get_file_type(src_file , &file_type , &fmt_file , &report_nr);
	if (file_type == ecl_other_file) {
	  fprintf(stderr,"File: %s - problem \n",src_file);
	  fprintf(stderr,"In a list of many files ALL must be recognizable by their name. \n");
	  exit(1);
	}
	util_alloc_file_components(src_file , &path , &basename , &extension);
	
	target_file = ecl_util_alloc_filename(path, basename , file_type , !fmt_file , report_nr);
	file_convert(src_file , target_file , file_type , fmt_file);
	
	free(path);
	free(basename);
	free(extension);
	free(target_file);
      }
    }
    return 0;
  }
}
