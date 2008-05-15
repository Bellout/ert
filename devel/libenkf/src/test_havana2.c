#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <havana_fault_config.h>
#include <havana_fault.h>
#include <path_fmt.h> 
#include <util.h>


int main(int argc, char ** argv) 
{

  if (argc < 4) {
    printf("Usage: test_havana   <directoryprefix> <config file>  <nreal>\n");
    exit(1);
  }

  /*

  o Ikke bland variabler og kode - det er ryddig og bra med variabler
    som har begrenset leksikalt skope, men bruk i det tilfellet { }
    for � tydeliggj�re skopet.

  o I slike opplistinger som er gitt nedenfor s� b�r vi ta oss tid til
    � opplinjere p� "=" - det �ker lesbarheten.

  o Jeg har startet med 'directory_prefix' konvensjonen; jeg skal ikke
    argumentere for at den ene konvensjonen er bedre enn den
    andre. Men, jeg vil argumentere TUNGT for at en konvensjon er
    bedre enn flere. Jeg ber derfor om at vi ikke bruker computerCaps.

  o atoi() er selvf�lgelig OK, men funksjonen util_sscanf_int() er
    basert p� funksjonen strtol() som er en forbedring av atoi(), den
    gir ogs� (mulighet for) feilh�ndtering.

  o Noen ganger er det selvf�lgelig sv�rt fristende � allokere en
    streng som er "lang nok"; i dette tilfellet b�r det ikke v�re
    n�dvendig: 
     * Bruk strlen() til � allokere en streng av riktig lengde.
     * Bruk util_malloc() til � allokere - da f�r du automatisk en
       sjekk p� at allokeringen gikk bra.
     * Ikke cast retur-verdien fra malloc() - det gir ingenting.
     * N�r det gjelder filnavn / kataloger s� fors�k � bruke
       util_path_xxx() relaterte funksjoner, da blir det (eventuelt)
       mye lettere � porte til windows p� et senere tidspunkt.
       * Variabelen format skal kun leve som input til en path_fmt
       instans - den b�r defineres som en variabel med begrenset
       skope. 
  */

  {
    const char * directory_prefix    		 = argv[1];
    const char * config_file                     = argv[2];
    const char * ens_size_string                 = argv[3];

    int    ens_size;

    path_fmt_type             * run_path_fmt;
    havana_fault_type        ** ensemble; 
    havana_fault_config_type  * havana_config;

    if (!util_sscanf_int(ens_size_string , &ens_size)) 
      util_abort("%s: Failed to interpret:%s as an integer - exiting.\n",__func__ , ens_size_string);
    
    {
      const char * file_fmt = "ens%d";
      char       * format   = util_alloc_full_path(directory_prefix , file_fmt);
      run_path_fmt = path_fmt_alloc_directory_fmt(format,true);
      free(format);
    }

    havana_config =  havana_fault_config_fscanf_alloc(config_file);
    ensemble      =  util_malloc(ens_size * sizeof *ensemble , __func__);
    {
      /* 
        Bruk leksikalt begrensede telle-variabler, fors�k � gi
        tellevariblene et navn som indikerer hva de teller over. Det
        er for eksempel �penbart at 'iens' teller over ensemble
        medlemmer, 'i' derimot kan telle over hva som helst. 
      */
      int iens;

      for (iens=0; iens < ens_size; iens++) {
	/*
	  Fors�k � organisere malloc / free i en n�stet struktur, slik
	  at det f�rste objektet som blir allokert i et scope, er det
	  siste som blir free'et:

          char * target_file = path_fmt_alloc_file() <----�
	  ensemble[iens] = havana_fault_alloc(); <----�   |
          ....                                        |   |
          ....                                        |   |
          havana_fault_free(ensemble[iens]);  <-------�   |
	  free(target_file); <----------------------------� 
	*/

	
	char * target_path = path_fmt_alloc_path(run_path_fmt , iens+1);
	ensemble[iens] = havana_fault_alloc(havana_config);
	havana_fault_initialize(ensemble[iens],iens);
	havana_fault_ecl_write(ensemble[iens] , target_path);
	havana_fault_free(ensemble[iens]); 
	free(target_path);
	
      }
    }
    free(ensemble);
    havana_fault_config_free(havana_config);  
    path_fmt_free(run_path_fmt);
  }
}   



