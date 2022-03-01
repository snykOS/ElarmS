C ----------------------------------------------------------------------
      SUBROUTINE WHERE (EQLAT, EQLON, IRES)

C   Given the lat and lon of a point write strings containing the
c   nearest geographic location for each group as designated by the
c   first character in the listing ('grp').
c
c NOTE: Sign of the longitude is significant! We assume negative longitude is
c	in the western hemisphere.
c
c	Ex. "  14 miles southwest of PALMDALE"
c	    "  30 miles N     of LOS ANGELES"
c	    "  36 miles Nwest of PASADENA"

c	Information about towns is in an ASCII data file equated with the
c	logical name DCK$TOWN. The format is (i2, 1x, f6.3, 1x, f8.3, 1x, a50)
c	as follows:
c
c	iiXff.fffXffff.fffXaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa
c	 1 34.223 -118.448 PANORAMA CITY          
c	01 34.143 -114.287 PARKER                   
c	 2 35.742 -120.350 PARKFIELD                
c	 3 34.145 -118.140 Site of some past important earthquake
c	 4 32.65  -114.65  PLT (blast)              
c	etc.
c
c	The leading number assigns each town to a group.  The closest town in 
c	EACH group is output, therefore there are as many lines output as there 
c	are groups.  This way you can be assured of not only finding the 
c	closest town on the list but also the closest MAJOR city, the closest
c	quarry site or whatever you choose.

c	Our current scheme is:
c	Group 1		smaller towns, landmarks
c	Group 2		large towns
c	Group 3		Pasadena
c	Group 4		quarry sites
c	Group 5		notable earthquakes or sequences

c	IRES returns the number of groups (and output strings) if successful
c
c	The DCK$TOWN is only read the first time this routine is called
c	When it is read the data is stored in arrays for future use
c
C MODIFICATION HISTORY:
c 12-FEB-1992 DDG - to pass location description strings through
c	common /WHERE_OUT/ so they can be used by the program, don't write
c	anything out from subroutine, let the calling program do it
c
c 7-JUL-1992 DDG - make abreviated azimuth descripton "NNW", etc. 
c	rather then "NORTH-NORTHWEST"

c 4-JAN-1995 DDG - made where.inc a structure, added logic so WHERE 
c	wouldn't look up the same location over again if called again
c
c 28-APR-1998 DDG - moved reading of DCK$TOWN to a subroutine
c
c .. arrays holding all the town data (is read in the first time)
      
      integer lat, lon
      real	xlat, xlon
      
c .. arrays holding index of closest 'town' in each group 
      
      include 'where.inc'       !MAX_GRPS is defined here
      
      integer*4 	iclosest(MAX_GRPS) !index (number) of closest town
      real*4 		fmin(MAX_GRPS) !dist to closest town for this group
      
      character*15 cdir(16)     ! compass points
      data cdir /'N',
     $     'NNE',
     $     'NE ',
     $     'ENE',
     $     'E  ',
     $     'ESE',
     $     'SE ',
     $     'SSE',
     $     'S  ',
     $     'SSW',
     $     'SW ',
     $     'WSW',
     $     'W  ',
     $     'WNW',
     $     'NW ',
     $     'NNW'/
      
c ... zero things out
      do i = 1, MAX_GRPS
         where_rec.cwhere(i) = ' '
         fmin(i) = 10000.0      !max out the minimums
      end do
      
c ****** read in town data, if this is the first call to subroutine

      if (town.ntowns .eq. 0) then
         
         call read_town_dck (paths.townfile, IRES)
         if (ires .lt. 0) then  !981109 -rsd
	    type *, 'Where_Point/Read_Town_Dck error. IRES= ', IRES, !981109 -rsd !981109 -rsd
     $           ' Try Read_Town_Dck_Decimal'				
	    call read_town_dck_decimal (paths.townfile, IRES) !981109 -rsd
	    if (IRES .GE. 1) then !981109 -rsd
               type *, 'Where_Point/Read_Town_Dck_Decimal SUCCESS' !981109 -rsd
     $              
	    endif               !981109 -rsd
         endif                  !981109 -rsd
         if (ires .lt. 0) return
         
      end if
      
c ......................................................................
c .. find the closest town
c .. test to see if each town is closer than the previous closest for the group
c	(first cut checking is done in degrees, this is approximate but
c	 is quicker than calculating km. The real distance in km will be
c	 calculated only for the closest town in each group)

      do i = 1, town.ntowns     !for each town
         
         dlat = abs(EQLAT - town.slat(i)) !lat difference
         if (dlat .gt. fmin(town.ngroup(i))) goto 390 !farther than previous in group
         
         dlon = abs(EQLON - town.slon(i)) !lon difference (west lon is neg!)
         if (dlon .gt. fmin(town.ngroup(i))) goto 390 !farther than previous
         
         ddist = sqrt(dlat*dlat + dlon*dlon) !'diagonal' dist in degrees
         if (ddist .gt. fmin(town.ngroup(i))) goto 390 !farther than previous
         
c .. this town is closer
         iclosest(town.ngroup(i)) = i !reset test values to latest
         fmin(town.ngroup(i)) = ddist !set min to new minimum
 390  end do
      
c ......................................................................
c .. all out of towns, put results in the common strings

      do i = 1, where_rec.ngrps !output a line for each group
         call dist_az (eqlat, eqlon, 
     $        town.slat(iclosest(i)), town.slon(iclosest(i)), dist, az)
         
         idist = nint(dist * 0.6214) !convert to integer miles (round)
         ikdist = nint(dist)    !convert to integer km (round)
         
c .. figure out which of sixteen directions quake is in from town
c	each hexant is 22.5 deg., hexant #1 is north and they are arranged
c	clockwise from there

         taz = az + 11.25
         if (taz .ge. 360.) taz = taz - 360.	
         
         i_hexant = int(taz/22.5) + 1
         
c .. put the string in the common array
         ier = 200 + i
         
         write (where_rec.cwhere(i), 110, iostat=lstat, err=910) idist,
     $        ikdist,cdir(i_hexant), town.name(iclosest(i)), char(0)
 110     format (i3, ' mi. (', i3, ' km) ', a3, ' of ', a50,a1)
         
      end do			!end of output loop
      
      ires = where_rec.ngrps    !return a good ires
      
      return

c --- ERROR path

 910  type*,'ERROR --  WHERE_POINT --- ', ier, ires, lstat !added _POINT	!981109 -rsd
      ires = - ier
      return
      end
      
C ----------------------------------------------------------------------
      SUBROUTINE WHERE_FAULT (EQLAT_IN, EQLON_IN, IRES)

c	DDG - 19-AUG-1992
C	Given the lat and lon of a point write a string containing the
c	nearest fault
c
c	Ex. "  14 miles southwest of San Jacinto fault"
c
c	Information about faults is in ASCII .GEO files as created by SIFT
c	File is defined by logical DCK$FAULT
c
c NOTE: The .GEO files use positive longitudes therefore we must take abs() of 
c	longitude here. Another testiment to Menlo's provinciality.
c
c DDG 14-JUN-1999 - Peg Johnson pointed out that azimuths are wrong sometimes. I 
c think this is a result of algorithm being tested only with negative longitudes.
c simplest fix was to force all longitudes to negative here.

c .. arrays holding all the town data (is read in the first time)
      PARAMETER 	(MAX_FAULTS = 500)
      PARAMETER	(MAX_PTS    = 50000)
      
      STRUCTURE/FAULT_DATA/
         character*50 	CNAME   !name of each fault
         integer*4	istart  !start index in slat/slon array
         integer*4	npts    !number of point/pairs for each fault
      END STRUCTURE
      
      real*4 		slat(max_pts), slon(max_pts) !points
      
      RECORD/FAULT_DATA/	FLT(MAX_FAULTS)
      
      integer*4 	nflt /0/ !number of faults
      
      include 'where.inc'
      
      character*15 cdir(16)     ! compass points
      data cdir /'N',
     $     'NNE',
     $     'NE ',
     $     'ENE',
     $     'E  ',
     $     'ESE',
     $     'SE ',
     $     'SSE',
     $     'S  ',
     $     'SSW',
     $     'SW ',
     $     'WSW',
     $     'W  ',
     $     'WNW',
     $     'NW ',
     $     'NNW'/

c DDG 14-JUN-1999 

      eqlat = eqlat_in
      eqlon = -1.0 * (ABS(eqlon_in))
      
c ****** read in fault data, if this is the first call to subroutine

      if (nflt .eq. 0) then 	!don't read in faults if it was done before

c .. open 'DCK$FAULT' (it is a logical)
         iunit = 10
         ier = 10
         open (iunit, 
     $        file = paths.faultfile,
     $        status='old', 
     $        readonly, 
     $        err=910, 
     $        blank='ZERO')     !see NOTE: below

c NOTE: .GEO file format has lat/lon pairs with spaces in place of leading 
c	zeros in the decimal portion. EX: "325823118 106" therefore, we must
c	interpret spaces as zeros.

         nflt     = 0		!number of faults read in
         npts_tot = 0		!total number of point/pairs for all faults
         
c .. read loop
  10	continue
        npts = 0		!total number of point/pairs for this fault
        nflt = nflt + 1
        flt(nflt).istart = npts_tot + 1 
        
        if (nflt .gt. max_faults) then !check for array overflow
           type*,' ** WHERE_FAULT WARNING: number of faults exceeds ', 
     $          max_faults ,' (proceeding..)'
           goto 300
        end if
        
        ier = 1000 + nflt
        
c ... read fault name line
        read (iunit, 100, iostat=lstat, err=910, end=300) 
     $       flt(nflt).cname
 100    format (78x, a50)
        
c There are up to 6 point/pairs per line. A block of data for a fault is 
c terminated by " 0   0  0   0" in any position in the line.
c There may be more than one block of points for a given fault but that makes no
c difference here so I ignore it.

  20    continue
c	... read next 6 fault point-pairs
        read  (iunit, 200, iostat=lstat, err=910, end=300) 
     $       (slat(j), slon(j), j=npts_tot+1, npts_tot+6)
 200    format (6(f6.4, f7.4))
        
        do i = npts_tot+1, npts_tot+6 !check for zeros
           if (slat(i) .gt. 0.0 .and. slon(i) .gt. 0.0) then
	      npts = npts + 1
	      npts_tot = npts_tot + 1
	      flt(nflt).npts = npts
              
              slon(i) = -ABS(slon(i)) !DDG 14-JUN-1999
              
           else
	      goto 10           !end of data for this fault, get another
           end if
           
        end do
        
        goto 20			!get another line

c ... end of read loop
 300	continue

        close (iunit)
        
        nflt = nflt - 1		!correct increment count of EOF
        
c	  type*, nflt, ' faults read in,', npts_tot,' total points.'

      end if

c ......................................................................
c .. find the closest point
c .. test to see if each point is closer than the previous closest 
c	(first cut checking is done in degrees, this is approximate but
c	 is quicker than calculating km. The real distance in km will be
c	 calculated only at the end)

      fmin = 10000.0
      
      do i = 1, nflt            !for each fault
         
         do k = flt(i).istart, flt(i).istart + flt(i).npts - 1
            
	    dlat = abs(EQLAT - slat(k)) !lat difference
            
	    dlon = abs(EQLON - slon(k)) !lon difference 
            
	    ddist = sqrt(dlat*dlat + dlon*dlon) !'diagonal' dist in degrees
            
	    if (ddist .lt. fmin) then !closer than previous closest
               klose_flt = i    !remember close fault and point
               klose_pt  = k
               fmin = ddist     !set min to new minimum
	    end if
            
         end do
      end do

c ......................................................................
c .. all done, output the results

      call dist_az (eqlat, eqlon, 
     $     slat(klose_pt), slon(klose_pt), 
     $     dist, az)
      
      idist = int(dist * 0.6214) !convert from km to integer miles
      ikdist = nint(dist)
      
c .. figure out which of sixteen directions quake is in from town
c	each hexant is 22.5 deg., hexant #1 is north and they are arranged
c	clockwise from there

      taz = az + 11.25
      if (taz .ge. 360.) taz = taz - 360.	
      
      i_hexant = int(taz/22.5) + 1

c .. put the string into "cfault"
      ier = 200 
      write (where_rec.cfault, 110, iostat=lstat, err=910) idist,
     $     ikdist,cdir(i_hexant), flt(klose_flt).cname, char(0)
 110  format (i3, ' mi. (', i3,' km) ', a3, ' of the ', a50, a1)
      
      ires = 1
      
      return

c --- ERROR path

 910  continue
      type*,'ERROR --  WHERE_FAULT --- ', ier, ires, lstat
      ires = - ier
      return
      end
      
c ----------------------------------------------------------------------
      SUBROUTINE DIST_AZ (plat1, plon1, plat2, plon2, dist, az)
      
c       Given two points figure the distance between them and the azimuth
c       from point 2 to point 1

      PARAMETER (R = 6371.0,    !radius of the earth
     $     FAC = 0.01745329)    !degrees to radians
      

c --    convert to radians (don't convert (contaminate) original values)

      DLAT = (plat1 - plat2) * FAC
      
      olat = plat2 * FAC
      olon = plon2 * FAC
      
      rlon = plon1 * FAC
      
      Y = R * DLAT
      X = (RLON - OLON) * R * COS((DLAT/2.0)+OLAT)
      
      dist = sqrt(x*x + y*y)
      
      if (x .eq. 0.0 .and. y .eq. 0.0) then
         az = 0.0
      else
         az = atan2d (x, y)
      end if
      if (az .lt. 0.0) az = az + 360.0
      
      return
      end
      

c ----------------------------------------------------------------------------

      SUBROUTINE READ_TOWN_DCK (CFILE, IRES)

c  28-APR-1998 DDG- created this sub

      include 'where.inc'       !MAX_GRPS is defined here

      character*(*) cfile

c .. open file 
      iunit = 10
      ier = 10
      open (iunit, 
     $     file=cfile, 
     $     status='old', 
     $     readonly, 
     $     err=910)
      
      town.ntowns = 0

c .. read loop

c NEW dck format - DDG 27-MAR-1998 
C|12345678901234567890123456789012345678901234567890123456789012345678901234567890
c|  1 34 15.9 -116 51.2 Big Bear City, CA
c|  1 35 22.4 -119  0.1 Bakersfield, CA
c|  3 35 25.2 -120 34.2 SANTA-MARGARITA (quarry)
c|  4 37 32.5 -118 26.6 Chalfant Valley EQ's (20,21 JUL 1986, 5.5, 6.4)
c|iiixiixffffxiiiixffffxaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa

c .. read loop
 10   town.ntowns = town.ntowns + 1

      read (iunit, 7100, err=920, end=200) 
     $     town.ngroup(town.ntowns), lat, xlat, lon, xlon, 
     $     town.name(town.ntowns)
 7100 format (i3, 1x, i2, 1x, f4.1, 1x, i4, f4.1, t23, a)
      
      town.slat(town.ntowns) =     lat  + (xlat/60.0)
      town.slon(town.ntowns) = ABS(lon) + (xlon/60.0)
      town.slon(town.ntowns) = sign(town.slon(town.ntowns),REAL(lon)) !preserve sign
      
      if (town.ntowns .gt. MAX_TOWNS) then !check for array overflow
C -RSD	    type*,' ** WHERE WARNING: number of towns exceeds ', 
         type*,' ** READ_TOWN_DCK WARNING: number of towns exceeds ', !981109 -RSD
     $        MAX_TOWNS ,' (Proceeding..)'
         goto 200
      end if
      
      if (town.ngroup(town.ntowns) .gt. MAX_GRPS) then !check for array overflow
C -RSD	    type*,' ** WHERE WARNING: number of town groups exceeds ', 
         type*,                 !981109 -RSD
     $        ' ** READ_TOWN_DCK WARNING: number of town groups exceeds
     $        ',MAX_GRPS, ' (Proceeding..)'
         goto 200
      end if
      
      where_rec.ngrps = max(where_rec.ngrps, !find highest group number
     $     town.ngroup(town.ntowns))
      goto 10                   !end of read loop
      
 200  continue
      
      close (iunit)
      
      town.ntowns = town.ntowns - 1 !correct increment count of EOF
      
      ires = town.ntowns
      
      return

c ... Error path
 910  continue
c      write (6, 7910) cfile(:lentrue(cfile))
C-RSD 7910	format (' ** WHERE ERROR: error opening file ', a)
 7910 format (' ** READ_TOWN_DCK ERROR: error opening file ', a) !981109 -RSD
      ires = -1
      
      return
      
 920  continue
c	write (6, 7920) cfile(:lentrue(cfile))
C-RSD 7920	format (' ** WHERE ERROR: error reading file ', a)
 7920 format (' ** READ_TOWN_DCK ERROR: error reading file ', a) !981109 -RSD
      ires = -1

      return
      end

c ----------------------------------------------------------------------------

      SUBROUTINE READ_TOWN_DCK_DECIMAL (CFILE, IRES)

c  28-APR-1998 DDG- created this sub

      include 'where.inc'       !MAX_GRPS is defined here

      character*(*) cfile

c .. open file 
      iunit = 10
      ier = 10
      open (iunit, 
     $     file=cfile, 
     $     status='old', 
     $     readonly, 
     $     err=910)
      
      town.ntowns = 0

c .. read loop

c OLD dck format - DDG 27-MAR-1998 
C|12345678901234567890123456789012345678901234567890123456789012345678901234567890
c| 1 35.363 -119.013 BAKERSFIELD

c .. read loop
 10   town.ntowns = town.ntowns + 1

      read (iunit, 7100, err=920, end=200) 
     $     town.ngroup(town.ntowns), town.slat(town.ntowns), 
     $     town.slon(town.ntowns), town.name(town.ntowns)
 7100 format (i2, 1x, f6.3, 1x, f8.3, 1x, a50)
      
      if (town.ntowns .gt. MAX_TOWNS) then !check for array overflow
C -RSD	    type*,' ** WHERE WARNING: number of towns exceeds ', 
         type*,                 !981109 -RSD
     $        ' ** READ_TOWN_DCK_DECIMAL WARNING: number of towns !981109 -RSD
     $        exceeds ',MAX_TOWNS ,' (Proceeding..)'
         goto 200
      end if
      
      if (town.ngroup(town.ntowns) .gt. MAX_GRPS) then !check for array overflow
C -RSD	    type*,' ** WHERE WARNING: number of town groups exceeds ', 
         type*,                 !981109 -RSD
     $        ' ** READ_TOWN_DCK_DECIMAL WARNING: number of town groups !981109 -RSD
     $        exceeds ',MAX_GRPS, ' (Proceeding..)'
         goto 200
      end if
      
      where_rec.ngrps = max(where_rec.ngrps, town.ngroup(town.ntowns)) !find highest group number
     $     
      goto 10                   !end of read loop
      
 200  continue

      close (iunit)

      town.ntowns = town.ntowns - 1 !correct increment count of EOF

      ires = town.ntowns

      return

c ... Error path
 910  continue
c      write (6, 7910) cfile(:lentrue(cfile))
C-RSD 7910	format (' ** WHERE ERROR: error opening file ', a)
 7910 format (' ** READ_TOWN_DCK_DECIMAL ERROR: error opening file ', !981109 -RSD
     $     a) 
      ires = -1
      
      return

 920  continue
c	write (6, 7920) cfile(:lentrue(cfile))
C-RSD 7920	format (' ** WHERE ERROR: error reading file ', a)
 7920 format (' ** READ_TOWN_DCK_DECIMAL ERROR: error reading file ', !981109 -RSD
     $     a) 
      ires = -1
      
      return
      end

c
      SUBROUTINE get_where_grps(str_array)
      
      include 'where.inc'

c
c This copies the contents of chwhere and cfault into a variable
c and returns.
c
      character*80 str_array(max_grps)
      
      do i = 1, max_grps
         str_array(i) =  where_rec.cwhere(i)//char(0)
      end do
      end
c
c 
c
      SUBROUTINE get_where_fault(str)
      
      include 'where.inc'
      
c
c This copies the contents of chwhere and cfault into a variable
c and returns.
c
      character*80 str
      
      str =  where_rec.cfault//char(0)
      return
      end
      

c
      SUBROUTINE set_paths(tfile, ffile)
      
      include 'where.inc'
      
c
c This sets the paths to the town file and fault file
c and returns.
c
      character*80 tfile
      character*80 ffile
      
      paths.townfile = tfile
      paths.faultfile = ffile
      end
      
