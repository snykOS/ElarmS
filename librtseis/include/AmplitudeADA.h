/***********************************************************

File Name :
	AmplitudeADA.h

Programmer:
	Phil Maechling

Description:
	This is the structure of the data in the ada buffer.

Creation Date:
	26 June 1996


Usage Notes:



Modification History:
	10 Mar 2003 - paulf - added fullwin field
	25 Mar 2003 - paulf - added flags and SNR fields
	10 Dec 2007 - Pete Lombard - Modified to use leap-second-aware 
		      tntime classes.

**********************************************************/
#ifndef amp_H
#define amp_H

struct reading_type
{
  unsigned int samples_after_start_time;
  float    data_point;
  float    quality;
};


struct wa_amp_reading_type
{
  unsigned int samples_after_start_time;
  float    uncorrected_data_point;
  float    corrected_data_point;
  float    quality;
};


struct amplitude_data_type
{
  struct reading_type        peak_acceleration;
  struct reading_type        peak_velocity;
  struct reading_type        peak_displacement;
  struct reading_type        spectral_peak_0_3;
  struct reading_type        spectral_peak_1_0;
  struct reading_type        spectral_peak_3_0;
  struct wa_amp_reading_type peak_wa_amp;
  float                      integral_of_velocity_squared;
  float                      energy_in_ergs;
  float                      ml100;
  float                      me100;
  int                        on_scale;
  int			     fullwin;	/* 0 if partial data used to fill struct, 1 if full */
  int 			     flags;	/* creation flags */
  float			     noise_variance;	/* avg peak values over window CGS units (vel or acc)*/
  float			     lta;   /* cummulative LTA window of noise (not including this one) CGS */
  float			     lta_window; /* size of LTA window above in seconds length */
  float			     snr;  /* CGS peak value of data over lta member */
};

// for the fullwin field
#define ADA_FULLWINDOW 1
#define ADA_SHORTFILL  0

// for the flags field
#define ADA_RAD_FLAG_NULL 		0x00
#define ADA_RAD_FLAG_FIRST_ADA 		0x01 
#define ADA_RAD_FLAG_FILTER_RESET 	0x02

#endif
