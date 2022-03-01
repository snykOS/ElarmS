/***********************************************************

File Name :
        CISN_MlAo.h

Programmer:
        Pete Lombard

Description:
        Header file describing the CISN_MlAo and CISN_la100 routines.

Creation Date:
        4 April 2007


Usage Notes:



Modification History:



**********************************************************/
#ifndef CISN_MLAO_H
#define CISN_MLAO_H

#ifdef __cplusplus

extern "C"
{
    double cisn_mlao_( double* hypodist );

    double cisn_la100_( );
}

#else

    double cisn_mlao_( double* hypodist );

    double cisn_la100_( );

#endif
#endif
