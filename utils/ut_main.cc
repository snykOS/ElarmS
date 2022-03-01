// $Id: ut_main.cc $
// 
// generic main for self contained unit tests
// intended for use with the auto-generated property files

#include HEADER
#include <stdlib.h> // exit

int main(int argc, char* argv[])
{
   int rc = CLASS::main(argc, argv);
   exit(rc);
} // main

// end of file ut_main.cc
