      SUBROUTINE GREAT(OLAT1,OLON1,OLAT2,OLON2,ODIST,ODISD,OGC,
     *  OZ12,OZ21)
      IMPLICIT REAL*8 (A-H,P-Z)
      ALAT1=OLAT1
      ALAT2=OLAT2
      ALON1=OLON1
      ALON2=OLON2
       ATH=6378.388
      BTH=6356.912
      PI = 3.14159265
      RAD = PI/180.
      H = 1. - BTH*BTH/(ATH*ATH)
      P = H/(1. - H)
      GR = ALON1*RAD
      TR = ALAT1*RAD
      SINTR =DSIN(TR)
      COSTR =DCOS(TR)
      IF (SINTR .EQ. 0.) SINTR = .00000100
      IF (COSTR .EQ. 0.) COSTR = .00000100
      R1 = ATH/DSQRT(1. - H*SINTR*SINTR)
      Z1 = R1*(1. - H)*SINTR
      G = ALON2*RAD
      T = ALAT2*RAD
      IF (T .EQ. 0.) T = .0000100
      SINT =DSIN(T)
      COST =DCOS(T)
      R2 = ATH/DSQRT(1. - H*SINT*SINT)
      DG = G - GR
      COSDG =DCOS(DG)
      SINDG =DSIN(DG)
      DGR = GR - G
      DT = T - TR
      Q = SINT*COSTR/((1. + P)*COST*SINTR) + H*R1*COSTR/(R2*COST)
      X = R2*COST*COSDG
      Y = R2*COST*SINDG
      Z = R2*(1. - H)*SINT
      AZ12 =DATAN2(SINDG,(Q - COSDG)*SINTR)
      Q = SINTR*COST/(COSTR*SINT*(1. + P)) + H*R2*COST/(R1*COSTR)
      AZ21 = DATAN2(DSIN(DGR),SINT*(Q-DCOS(DGR)))
      COS12 =DCOS(AZ12)
      CTA2 = COSTR*COSTR*COS12*COS12
      P0 = P*(CTA2 + SINTR*SINTR)
      B0 = (R1/(1. + P0))*DSQRT(1. + P*CTA2)
      E0 = P0/(1. + P0)
      GC = 2.*PI*B0*DSQRT(1. + P0)*(1. - E0*(.25 + E0*(3./64.
     *                                          + 5.*E0/256.)))
      C0 = 1. + P0*(.25 - P0*(3./64. - 5.*P0/256.))
      C2 = P0*(-.125 + P0*(1./32. - 15.*P0/1024.))
      C4 = (-1./256. + 3.*P0/1024.)*P0*P0
      U0 =DATAN2(SINTR,COSTR*COS12*DSQRT(1. + P0))
      U =DATAN2(R1*SINTR + (1. + P0)*(Z - Z1),(X*COS12 - Y*SINTR*
     *                                       DSIN(AZ12))*DSQRT(1. + P0))
      DISD = U - U0
      IF (U .LT. U0) DISD = PI + PI + DISD
      DIST = B0*(C0*( DISD ) +C2*(DSIN(U + U) -DSIN(U0 + U0))
     *                       +C4*(DSIN(4.*U) -DSIN(4.*U0)))
      DISD = DISD/RAD
      AZ12 = AZ12/RAD
      AZ21 = AZ21/RAD
      IF (AZ12 .LT. 0.) AZ12 = 360. + AZ12
      IF (AZ21 .LT. 0.) AZ21 = 360. + AZ21
      ODIST=SNGL(DIST)
      ODISD=SNGL(DISD)
      OGC=SNGL(GC)
      OZ12=SNGL(AZ12)
      OZ21=SNGL(AZ21)
      RETURN
      END
 
