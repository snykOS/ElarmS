/***********************************************************

File Name :
	y2k.C

Programmer:
	Phil Maechling

Description:
	This is a y2k routine which applies a rule to determine
	if a 2 digit year is in the 20th or 21st Century.

Creation Date:
	24 August 1998


Usage Notes:



Modification History:



**********************************************************/


int find_four_digit_year(int short_year)
{

  if (short_year > 1000)
  {
    return(short_year);
  }
  else if (short_year < 50)
  {
    return (short_year + 2000);
  }
  else
  {
    return (short_year + 1900);
  }
}

int find_two_digit_year(int long_year)
{

  if (long_year < 1900)
  {
    return(long_year);
  }
  else if (long_year >= 2000)
  {
    return (long_year - 2000);
  }
  else
  {
    return (long_year - 1900);
  }
}

