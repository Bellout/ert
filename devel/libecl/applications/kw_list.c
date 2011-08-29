/*
   Copyright (C) 2011  Statoil ASA, Norway. 
    
   The file 'kw_list.c' is part of ERT - Ensemble based Reservoir Tool. 
    
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

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <ecl_kw.h>
#include <fortio.h>
#include <string.h>
#include <ecl_util.h>
#include <ecl_endian_flip.h>


void kw_list(const char *filename) {
  fortio_type *fortio;
  bool fmt_file = ecl_util_fmt_file(filename);

  printf("-----------------------------------------------------------------\n");
  printf("%s: \n",filename); 
  fortio = fortio_fopen_reader(filename , ECL_ENDIAN_FLIP , fmt_file);
  ecl_kw_type * ecl_kw = ecl_kw_alloc_empty();
  while(  ecl_kw_fread_realloc(ecl_kw , fortio) ) 
    ecl_kw_summarize(ecl_kw);
  printf("-----------------------------------------------------------------\n");

  ecl_kw_free(ecl_kw);
  fortio_fclose(fortio);
}


int main (int argc , char **argv) {
  int i;
  for (i = 1; i < argc; i++)
    kw_list(argv[i]);
  return 0;
}
