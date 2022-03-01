/***********************************************************

File Name :
        Compat.h

Original Author:
        Patrick Small

Description:

Creation Date:
        4 August 1998

Modification History:


Usage Notes:


**********************************************************/

#ifndef compat_H
#define compat_H

// Various include files


class Compat
{

 private:


 public:

    // Default Constructor
    //
    Compat();

    // Destructor
    //
    ~Compat();

    static char* Form(const char *strf, ...);

};


#endif

