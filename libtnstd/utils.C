/* CAUTION: THIS FUNCTION PRODUCES INCORRECT RESULTS FOR NEGATIVE NUMBERS!
   Pete Lombard, 2009/12/06
*/

#include "utils.h"


int utils::round(double a) {

  return int (a+0.5);

}
