c...  derived from the distaz program written
c...  by Tony Qamar, Dec. 1984.
c...  Computes distance and azimuth and p-arrival time from source location to an
c...  
      subroutine distaz(elat, elon, edepth, stalat, stalon, dist,
     1     azimuth, ptime)
      real elat, elon, edepth, stalat, stalon, ptime
      double precision pi,factr,rla1,cnla1,rlo1,rla2,cnla2,rlo2
      double precision rdeg,deg
      double precision geocen,delta,azim,height
      parameter (pi=3.141592653589793238d0)
      factr=pi/180.
      i=0

c...  A few Prelim calculations for source
      rla1=elat*factr
      if(abs(elat).ne.90.)then
         cnla1=geocen(rla1)
      else
         cnla1=rla1
      endif
      h1=height(cnla1)
      rlo1=elon*factr
c...  Minus sign changes UW longitude convention to standard one
      rlo1=-rlo1
c...  
c...  Calculate distance, Azimuth from source to site.
c...  
      rla2=stalat*factr
      rlo2=stalon*factr
c...  Minus sign changes UW longitude convention to standard one
      rlo2=-rlo2
      h2=height(rla2)
      if(abs(stalat).ne.90.)then
         cnla2=geocen(rla2)
      else
         cnla2=rla2
      endif
      rdeg=delta(cnla1,rlo1,cnla2,rlo2)
      azimuth=azim(rdeg,cnla1,cnla2,rlo1,rlo2)/factr
      deg=rdeg/factr
      ptime = ttlook(deg,edepth)
      dist = deg
      end
c...  
c...  returns p-wave arrival time given event distance (deg) and depth
c...  
      subroutine parrtime(deg, edepth, ptime)
      real deg, edepth, ptime
      double precision deg2
      deg2 = deg
      ptime = ttlook(deg2,edepth)
      end
c     
c............................................................
      double precision function geocen(rlat)
c...  returns geocentric latitude (radians) when the usual
c...  geographic latitude (rlat) is input in radians. Assumes
c...  earth ellipticity of 1/297.
c...  Routine doesnt work at rlat=+/-Pi.
c...  
c...  declare function to be double precision in calling program as in:
c...  double precision geocen .................
c...  
      double precision rlat,arg
      arg=0.993277*dtan(rlat)
      geocen=datan(arg)
      return
      end
c     
c.................................................................
      double precision function delta(rlat1,rlong1,rlat2,rlong2)
c...  given lat,long coords (radians) of points 1 and 2 routine
c...  returns geocentric angular distance (radians) between
c...  points 1 and 2. For best results be sure geocentric latitudes
c...  are fed to this routine. 
c...  
c...  declare function to be double precision in calling program as in:
c...  double precision delta .................
c...  
      double precision rlat1,rlong1,rlat2,rlong2,del
      del=dcos(rlat1)*dcos(rlat2)*dcos(rlong1-rlong2)
      del=del+dsin(rlat1)*dsin(rlat2)
c...  Account for roundoff
      if(del.gt.1.)del=1.
      if(del.lt.-1.)del=-1.
      delta=dacos(del)
      return
      end
c     
c...............................................................
      double precision function azim(delt,rla1,rla2,rlo1,rlo2)
c...  Given angular distance delt between points 1 and 2 and lat and
c...  long of points 1 and 2, azimuth from point 1 to 2 is returned.
c...  All angles (rla1, rla2, rlo1, rlo2, and azim) are in radians.
c...  Note:Azim is 0 to 2*PI clockwise from north. East longitude
c...  is + and west long. is -.
c...  
c...  declare function to be double precision in calling program as in:
c...  double precision azim .................
c...  
      double precision delt,rla1,rla2,rlo1,rlo2,pi,sdelt,phi
      parameter (pi=3.141592653589793238d0)
      parameter (epsln=1.e-7)
c...  Check special case.........................
      sdelt=dsin(delt)
      if(sdelt.lt.epsln)then
         azim=0.
         return
      endif
c...............................................
      if(rlo1.lt.0.)rlo1=rlo1+2.*pi
      if(rlo2.lt.0.)rlo2=rlo2+2.*pi
      nquad=0
      phi=rlo2-rlo1
      nchek=phi/pi+3.
      go to(10,20,30,40)nchek
 10   phi=phi+2.*pi
      nquad=1
      go to 50
 20   phi=-phi
      go to 50
 30   nquad=1
      go to 50
 40   phi=2.*pi-phi
 50   continue
      azim=-dcos(rla2)*dcos(phi)*dsin(rla1)
      azim=azim+dcos(rla1)*dsin(rla2)
      azim=azim/sdelt
c...  Account for roundoff
      if(azim.gt.1.)azim=1.
      if(azim.lt.-1.)azim=-1.
      azim=dacos(azim)
      if(nquad.eq.1)return
      azim=2.*pi-azim
      return
      end
c     
c...............................................................
      double precision function height(rlat)
c...  returns the value of the height (kms) of the elliptical
c...  earth above the Jeffreys mean spherical earth as a function
c...  of latitude (rlat, in radians). Assumes earth radius of 6378.38
c...  kms and ellipticity of 1/297.
c...  
c...  declare function to be double precision in calling program as in:
c...  double precision height .................
c...  
      double precision rlat
      height=-3.549+10.738*dcos(2.*rlat)-.023*dcos(4.*rlat)
      return
      end
c     
c................................................................
      function ttlook(del,depth)
c...  compute estimated P-travel time from given distance,
c...  del in degrees using an interpolated J-B TT table.
      double precision del
      real d(23)
      data (d(i),i=1,23) /5.0,74.1, 142.2, 207.9, 273.1, 324.2,
     1     371.6, 414.9, 456.7, 497.4, 534.4,
     2     570.6, 605.7, 641.9, 671.1, 699.5,
     3     730.2, 755.8, 779.8, 799.8, 825.5, 849.3, 874.0/
c...  Use interpolation of TT tables for less than 110 degrees
      if(del .lt. 110) then
         x = 0.
         do 20 i=1,23
            if(del.lt.x) then
               j = i
               goto 21
            endif
            x = x+5.
 20      continue
 21      a = (del-x)/5.
         tttmp = d(j) + a * (d(j)-d(j-1))
      else
c...  PKP branch for more than 110 degrees
         tttmp = 1140. + 1.46 * (del-110.)
      endif
c     compensate for depth
      if (del.ge.15.and.depth.ge.100) then
         ttlook = tttmp - (del * depth)/1000. - 10
      else
         ttlook = tttmp
      endif
      return
      end
