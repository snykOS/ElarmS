c
c from Bob Urhammer May 8, 2007 email
c
	real*8 function CISN_mlAo( rdist )
c
c ...... calculate CISN -logAo ML attenuation function
c
	implicit none
	integer*4 j
	real*8 rdist, TP(6), mlogAo, T, z, x, b, logAo
c
	TP( 1 ) = +0.056d0
	TP( 2 ) = -0.031d0
	TP( 3 ) = -0.053d0
	TP( 4 ) = -0.080d0
	TP( 5 ) = -0.028d0
	TP( 6 ) = +0.015d0
c
	if( rdist .le. 0.1d0 ) then
c
c ...... invalid for rdist less that 0.1 km
c	 return with -9.d0
c
	  mlogAo = -9.d0
c
	elseif( rdist .le. 8.d0 ) then
c
c ...... linear extrapolation of average slope between 8 km and 60 km
c
	  b = ( 2.6182d0-1.5429d0)/(dlog10(60.d0)-dlog10(8.d0))
	  mlogAo = 1.5429d0 + b * ( dlog10( rdist ) - dlog10( 8.d0 ) )
c
	elseif( rdist .le. 500.d0 ) then
c
c ...... Chebychev polynomial expansion
c
	  x = z( rdist )
	  mlogAo = logAo( rdist ) + 0.0054d0
	  do j = 1 , 6
	    mlogAo = mlogAo + TP( j ) * T( j , x )
	  end do
c
	else
c
c ...... invalid for rdist greater than 500 km
c	 return with -9.d0
c
	  mlogAo = -9.d0
c
	endif
c
	CISN_mlAo = mlogAo
c
	return
	end

	real*8 function T( n , x )
c
c ...... Chebyshev Polynomial
c
	implicit none
	integer*4 n
	real*8 x, theta
c
	theta = dacos( x )
	T = dcos( dble( n ) * theta )
c
	return
	end

	real*8 function z( r )
c
c ...... translate scale from r to z
c
	integer*4 ncall
	real*8 r, r0, r1, z0, z1, a , b, l_r0, l_r1
c
	data ncall /0/
	data r0,r1 /8.d0,500.d0/
	data z0,z1 /-1.d0,+1.d0/
c
	save ncall, a, b
c
	if( ncall .eq. 0 ) then
	  l_r0 = dlog10( r0 )
	  l_r1 = dlog10( r1 )
	  b = ( z1 - z0 ) / ( l_r1 - l_r0 )
	  a = z0 - b * l_r0
	  ncall = 1
	endif
c
	z = a + b * dlog10( r )
c
	return
	end

	real*8 function logAo( rdist )
c
c ...... -logAo attenuation function
c
	implicit none
	real*8 rdist
	
c
	logAo = 1.11d0 * dlog10( rdist ) + 
     1    0.00189d0 * rdist + 0.591d0
c
	return
	end

