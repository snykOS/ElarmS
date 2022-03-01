/***********************************************************

File Name : 
	sgcdaf.C


Programmer:
	Phil Maechling

Description:
	Scan gcda ids file.
	Given a gcda id file name, returns the two ids.

Creation Date:
	17 September 1996


Usage Notes:



Modification History:
	27 August 1997
	Modified so that it works for cda and eda.

**********************************************************/
#include <iostream>
#include <cstdio>
#include <cstring>
#include "RetCodes.h"
#include "sgcdaf.h"

using namespace std;
extern int debug;

int find_gcda_ids(char* cfg_file,
                  char* in_station,
		  char* in_stream)
{
  FILE *cfgfp;
  cfgfp = fopen(cfg_file,"r");
  if (cfgfp == NULL)
  {

    std::cout << "Error opening the configuration file" << std::endl;
    std::cout << "Failed opening " << cfg_file << std::endl;
    return(TN_FAILURE);
  }

  char inputline[250];

  // Define scanned values

  int res;
  int res1;
  char* cres;
 
  int cda_ids_not_found = TN_TRUE;

  while (( !feof(cfgfp) ) && (cda_ids_not_found)) 
  {

    cres = fgets(inputline,sizeof(inputline),cfgfp);

    if (feof(cfgfp))
    {
      std::cout << "Found end of gcda ids configuration file" << std::endl;
      fclose(cfgfp);
      return(TN_FAILURE);
    }    

    if (cres == NULL)
    {
      std::cout << "Error scanning gcda ids configuration file" << std::endl;
      fclose(cfgfp);
      return(TN_FAILURE);
    }

    // Check for leading # 
    
    res = strncmp("#",inputline,1);
    if(res == 0)
    {
      // Found comment line
      continue;
    } 
   
    char station[10];
    char stream[10];

    res = sscanf(inputline,"%s %s",
	         station,
                 stream);
    
    if (res != 2)
    {
      std::cout << "Error scanning cda ids configuration file" << std::endl;
      fclose(cfgfp);
      return(TN_FAILURE);
    }
    else
    {
      strcpy(in_station,station);
      strcpy(in_stream,stream);
      cda_ids_not_found = TN_FALSE;
      fclose(cfgfp);

      std::cout << "Station       : " 
         << in_station << std::endl;
      std::cout << "Stream        : " 
         << in_stream
	 << std::endl;
        std::cout << "" << std::endl;
    }
  } // end reading configuration file

  return(TN_SUCCESS);

}
