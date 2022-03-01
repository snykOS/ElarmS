c/***********************************************************
c
cFile Name :
c
c
cProgrammer:
c	Phil Maechling
c
cDescription:
c
c
cCreation Date:
c
c
c
cUsage Notes:
c
c
c
cModification History:
c
c
c
c**********************************************************/
c


        subroutine adjave(x,n,av,sv,j,nj,thr)

c
c----  remove outliers to compute the average ----
c
c       reject outliers in x(i) using Chauvenet's criterion
c       and store the indices of rejected data in j(i)
c       thr is the threshold in Chauvenet's test, usually 0.5
c       Sept. 1996, Hiroo Kanamori
c
c	x will contain a list of real numbers.
c	on return x will be modified with only good data in it.
c	j will contain the indicies of the values that were dropped
c
c
c 
        dimension  x(*)
	dimension  j(*)
c
c av is the stdev of input array
c
	real av
c
c sv is the stdev of output array
c
	real sv
c
c j is number of rejected points
c
	integer j
	real	thr
c
c
c
c
        call stdv(x,n,av,dummy1,dummy2,sv)
        nj=1
        nsr=1
        do i=1,  n
        if(ichauv(x(i),av,sv,thr,n).eq.1) then
        j(nj)=i
        nj=nj+1
        go to  525
        end if
        x(nsr)=x(i)
        nsr=nsr+1
525     continue
        end do
        nsr=nsr-1
        nj=nj-1
        call stdv(x,nsr,av,dummy1,dummy2,sv)
        return
        end

      function  ichauv(x,xm,s,ant,n)
c     Chauvenet's data rejection criterion
c     if ichauv=1, consider rejection
c     if ichauv=0,  no reason for rejection
      dimension t(40), p(40)
      data t(1),t(2),t(3),t(4),t(5),t(6),t(7),t(8),t(9),t(10),
     2t(11),t(12),t(13),t(14),t(15),t(16),t(17),t(18),t(19),t(20),
     2t(21),t(22),t(23),t(24),t(25),t(26),t(27),t(28),t(29),t(30),
     2t(31),t(32),t(33),t(34),t(35)
     2/0.0,0.1,0.2,0.3,0.4,0.5,0.6,0.7,0.8,0.9,1.0,1.1,1.2,1.3,
     21.4,1.5,1.6,1.7,1.8,1.9,2.0,2.1,2.2,2.3,2.4,2.5,2.6,2.7,
     22.8,2.9,3.0,3.5,4.0,4.5,5.0/
      data p(1),p(2),p(3),p(4),p(5),p(6),p(7),p(8),p(9),p(10),
     2p(11),p(12),p(13),p(14),p(15),p(16),p(17),p(18),p(19),p(20),
     2p(21),p(22),p(23),p(24),p(25),p(26),p(27),p(28),p(29),p(30),
     2p(31),p(32),p(33),p(34),p(35)
     2/0.0,7.97,15.85,23.58,31.08,38.29,45.15,51.61,57.63,63.19,
     268.27,72.87,76.99,80.64,83.85,86.64,89.04,91.09,92.81,94.26,
     295.45,96.43,97.22,97.86,98.36,98.76,99.07,99.31,99.49,99.63,
     299.73,99.95,99.994,99.9993,99.99994/
      ichauv=0
      tm=abs((x-xm)/s)
      pm=ynterp(t,p,tm,35,0)/100.
      if (float(n)*(1.-pm).le.ant)  then
      ichauv=1
      end if
      return
      end
      
      subroutine  stdv(x,n,avx,var,std1,std2)
      dimension x(*)
      sum1=0.
      sum2=0.
      do i=1, n
      sum1=sum1+x(i)
      sum2=sum2+x(i)**2
      end do
      avx=sum1/float(n)
      var=(sum2-float(n)*avx**2)/float(n)
      std1=sqrt(var)
      std2=sqrt(var*float(n)/float(n-1))
      return
      end
