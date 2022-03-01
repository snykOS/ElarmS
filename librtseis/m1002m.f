c/***********************************************************
c
cFile Name :
c 	m1002m.f
c
cProgrammer:
c	Phil Maechling
c
cDescription:
c	This is a routine by Hiroo Kanamori to convert m100's to 
c       magnitudes.
c
c	Either ml or me is returned depending on the input flag.
c
cCreation Date:
c
c	8 December 1996
c
cUsage Notes:
c
c	Use a positive number for depth, not a negative number.
c
c
cModification History:
c
C The log A0 function for Ml, written by Hiroo Kanamori probably before
C 2000, has been superceded by the CISN log A0 function
C rtseis/CISN_MlAo.f 
c This code is still useful for Me for now. Pete Lombard, 2007/04/30
c
c**********************************************************/

      subroutine  m1002m(am100,am,dist,depth,iflag)
c
c
c Input paramters
c
	real	am100
	real	am
	real 	dist
	real	depth
	integer	iflag
c
c
c
c	
c     subroutine to convert m100 to m(dist, depth)
c     if iflag=0, it works for ML and if ifkag=1, it is for Me
c
c
c

      parameter(hr=8., dr=100., anl=1.2178, akl=0.0053,
     2ane=1.0322, ake=0.0035, c1=1.96)
c
c
c
c
      r=sqrt(dist**2+depth**2)
      rr=sqrt(dr**2+hr**2)
      c2=alog10(r/rr)
      c3=0.434*(r-rr)
      if (iflag.eq.0) then
      am=am100+anl*c2+akl*c3
        else if(iflag.eq.1)  then
        am=am100+2.*(ane*c2+ake*c3)/c1
         else
         write(*,*) 'iflag must be either 0 or 1'
          end if
      return
      end








      subroutine  m1002m_hyp(am100,am,dist,iflag)
c
c
c Input paramters
c
	real	am100
	real	am
	real 	dist
	integer	iflag
c
c
c
c	
c     subroutine to convert m100 to m(dist)
c     if iflag=0, it works for ML and if ifkag=1, it is for Me
c
c
c

      parameter(hr=8., dr=100., anl=1.2178, akl=0.0053,
     2ane=1.0322, ake=0.0035, c1=1.96)
c
c
c
c
c      r=sqrt(dist**2+depth**2)
      r=dist
      rr=sqrt(dr**2+hr**2)
      c2=alog10(r/rr)
      c3=0.434*(r-rr)
      if (iflag.eq.0) then
      am=am100+anl*c2+akl*c3
        else if(iflag.eq.1)  then
        am=am100+2.*(ane*c2+ake*c3)/c1
         else
         write(*,*) 'iflag must be either 0 or 1'
          end if
      return
      end
