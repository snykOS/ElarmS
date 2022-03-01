      real*8 function CISN_la100( )
c
c Return the CISN log_A0 value at 100 km epicentral distance
c and reference depth of 8 km.
c
      real*8 dist, rdepth, rdist, CISN_mlAo

      data dist,rdepth /1.d2,8.d0/

      rdist = dsqrt(dist * dist + rdepth * rdepth)
      CISN_la100 = CISN_mlAo(rdist)
      return
      end
