      subroutine travel_t(dist,p1,p2,s1,s2)
      
      real    dist,p1,p2,s1,s2

      DIMENSION  HL(50), V(50), T(100), DTDD(100), DTDH(100),
     &     AI1(100), AIH(100)

      ncall=1
      h = 0.0
      delt0 = dist
      ddelt = 0.0
      ndlt = 1
c     
c     First calculate the P wave travel times
c     
      NL = 5

      HL(1) = 1.0
      V(1)  = 4.0
      
      HL(2) = 3.0
      V(2)  = 5.5

      HL(3) = 23.4
      V(3)  = 6.3
      
      HL(4) = 5.0
      V(4)  = 6.8

      HL(5) = 9999.0
      V(5)  = 7.8

      AI=1
      DELT = AI*DDELT+DELT0 
      CALL  TDLT3( HL, V, H, DELT, T, DTDD, DTDH, AI1,
     &     AIH, NL, NCALL ) 
      call ordrjx(t,jt,nl)
c     
c     These tests are supposed to fix a problem when dist = 0.0. In
c     that case, this routine returns bad time values
c     

      if(t(1) .eq. 9999.9) then
         p1=0.0
      else
         p1=t(1)
      endif

      if(t(3) .eq. 9999.9) then
         p2 = 0.0
      else
         p2=t(3)
      endif
c     
c     
c     Now calculate S wave travel times
c     

      ncall=1
      h = 0.0
      delt0 = dist
      ddelt = 0.0
      ndlt = 1

      NL = 5

      HL(1) = 1.0
      V(1)  = 2.31

      HL(2) = 3.0
      V(2)  = 3.18

      HL(3) = 23.4
      V(3)  = 3.5

      HL(4) = 5.0
      V(4)  = 3.8

      HL(5) = 9999.0
      V(5)  = 4.33

      AI=1
      DELT = AI*DDELT+DELT0 
      CALL  TDLT3( HL, V, H, DELT, T, DTDD, DTDH, AI1, 
     &     AIH, NL, NCALL ) 
      call ordrjx(t,jt,nl)
c     
c     These tests are supposed to fix a problem when dist = 0.0. In
c     that case, this routine returns bad time values
c     

      if(t(1) .eq. 9999.9) then
         s1= 0.0
      else
         s1=t(1)
      endif

      if(t(2).eq.9999.9) then
         s2 = 0.0
      else
         s2=t(2)
      endif

      return

      END 
      SUBROUTINE TDLT3 (HL,V,H,DELT,T,DTDD,DTDH,AI1,AIH,NL,NCALL)
C     PROGRAM OF TRAVEL TIME CALCULATION FOR MULTI-LAYERED STRUCTURE
C     WRITTEN BY H.KANAMORI, OCTOBER,1972  (TDLT2)
C     HL=LAYER THICKNESS,V=VELOCITY,H=FOCAL DEPTH, DELT=DISTANCE,
C     T=TRAVEL TIME, DTDD=DT/DD, DTDH=DT/DH, AI1=INCIDENCE ANGLE AT
C     SURFACE, NL=NUMBER OF LAYERS, NCALL=1 WHEN CALLED 1ST. TIME,
C     NCALL>1 THEREAFTER
C     AIH=EMERGENCE ANGLE AT FOCUS
C     THE NL-TH LAYER IS HALF SPACE WHOSE THICKNESS NEED NOT BE GIVEN.
      DIMENSION  HL(1),V(1),T(1),DTDD(1),DTDH(1),AI1(1),AIH(1)
      DIMENSION  A(50,50),CLK(50,50),TLK(50,50)
      HL(NL)=9000.0
      DO 3 I=1,NL
         T(I) = 9999.9
         DTDD(I) = 9999.9
         DTDH(I) = 9999.9
         AI1(I) = 9999.9
         AIH(I) = 9999.9
    3 CONTINUE
      HS=0.0
      DO 1 I=1,NL
         HS =HS+HL(I)
         IF(H.GT.HS) GOTO 1
         K=I
         KM1=K-1
         GOTO 300
    1 CONTINUE
 300  HMHK=H-(HS-HL(K))
      IF(NCALL.GT.1) GOTO 301
      IF (NL.EQ.1) GOTO 301
      NLM1=NL-1
      DO 5 IK=1,NLM1
         IKP1=IK+1
         DO 6 IL=IKP1,NL
            A(IK,IL) = V(IK)/V(IL)
            CLK(IK,IL)=SQRT(1.0-A(IK,IL)**2)
            TLK(IK,IL)=A(IK,IL)/CLK(IK,IL)
 6       CONTINUE
    5 CONTINUE
 301  CONTINUE
      IF(K.GT.1) GOTO 310
      T(1)=(SQRT(H**2+DELT**2))/V(1)
      IF(T(1).LE.0.0) GOTO 28
      DTDD(1)=DELT/(T(1)*V(1)**2)
      DTDH(1)=H/(T(1)*V(1)**2)
      AI1(I)=ATAN2(DELT,H)
      AIH(I)=AI1(I)
      GOTO 320
 28   DTDD(1)=1.0/V(1)
      DTDH(1)=DTDD(1)
      GOTO 320
 310  X=DELT/H
 330  S1=0.0
      S2=0.0
      DO 15 IK=1,KM1
         Q=SQRT(1.0+(1.0-A(IK,K)**2)*X**2)
         S1=S1+HL(IK)*A(IK,K)*X/Q
         S2=S2+HL(IK)*A(IK,K)/Q**3
 15   CONTINUE
      G=HMHK*X+S1-DELT
      GP=HMHK+S2
      X=X-G/GP
      RES=ABS(G)-0.01
      IF(RES) 331,330,330
 331  CONTINUE
      Q2=SQRT(1.0+X**2)
      S1=0.0
      DO 16 IK=1,KM1
         Q=SQRT(1.0+(1.0-A(IK,K)**2)*X**2)
         S1=S1+HL(IK)*Q2/(V(IK)*Q)
 16   CONTINUE
      T(K)=HMHK*Q2/V(K)+S1
      DTDH(K)=1.0/(V(K)*Q2)
      DTDD(K)=X*DTDH(K)
      Q=SQRT(1.0+(1.0-A(1,K)**2)*X**2)
      AI1(K)=A(1,K)*X/Q
      AI1(K)=ATAN(AI1(K))
      AIH(K)=ATAN(X)
 320  CONTINUE
      IF(K.GE.NL) RETURN
      KP1=K+1
      DO 7 IL=KP1,NL
         ILM1=IL-1
         S1=0.0
         S2=0.0
         DO 8 IK=K,ILM1
            S1=S1+HL(IK)*TLK(IK,IL)
            S2=S2+HL(IK)/(CLK(IK,IL)*V(IK))
 8       CONTINUE
         S3=0.0
         S4=0.0
         DO 9 IK=1,ILM1
            S3=S3+HL(IK)*TLK(IK,IL)
            S4=S4+HL(IK)/(CLK(IK,IL)*V(IK))
 9       CONTINUE
         AL=DELT-(S1+S3-HMHK*TLK(K,IL))
         IF (AL) 7,351,351
 351     T(IL)=AL/V(IL)+S2+S4-HMHK/(V(K)*CLK(K,IL))
         DTDD(IL)=1.0/V(IL)
         DTDH(IL)=TLK(K,IL)/V(IL)-1.0/(V(K)*CLK(K,IL))
         AI1(IL)=ATAN(TLK(1,IL))
         AIH(IL)=ASIN(A(K,IL))
         IF(DTDH(IL).LT.0.0) AIH(IL)=3.14159-AIH(IL)
    7 CONTINUE
      RETURN
      END

      SUBROUTINE  ORDRJX ( X, J, N )
      DIMENSION X(*),J(*)
      J(1) = 1
      IF( N .EQ. 1 )  GO TO 320
      I = 2
 310  IF( I .GT. N )  GO  TO 320
      XTST = X(I)
      IM1 = I-1
      DO  300 JJ = 1, IM1
         K = I - JJ
         TST= X(K) - XTST
         IF( TST .GT. 0.0 )  GO TO  300
         KST = K + 1
         GO TO 330
 300  CONTINUE
      KST = K
 330  C=XTST
      IC = I
      DO 340 L=KST, I
         B = X(L)
         X(L) = C
         C = B
         IB = J(L)
         J(L) = IC
         IC = IB
 340  CONTINUE
      I = I + 1
      GO TO 310
 320  RETURN
      END
