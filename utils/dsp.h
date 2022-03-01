/** @file dsp.h
 * @brief This file implements basic digital signal processing functions
 */

#ifndef __dsp_h
#define __dsp_h

#include "butterworth_c.h"
//#include "wp.h"

namespace mathfn {
    // ground motion template functions
    /** Integration routine, recursive
    * @param x input array
    * @param y output (integrated) array, modified by this function
    * @param nsamp number of samples in array to be integrated
    * @param prevx previous last value in input array
    * @param prevy previous last value in integrated array
    * @param dt2 half array sampling period
    */
    template <class T>
    void integrate(T* x, T* y, size_t nsamp, T prevx, T prevy, T dt2) {
        y[0] = (x[0] + prevx)*dt2 + prevy;
        for (size_t i=1; i<nsamp; i++) {
            y[i] = (x[i] + x[i-1])*dt2 + y[i-1];
        }
    }
    /** Differentiation routine, recursive
    * @param x input array
    * @param y output (integrated) array, modified by this function
    * @param nsamp number of samples in array to be differentiated
    * @param prevx previous last value in input array
    * @param dt array sampling period
    */
    template <class T>
    void differentiate(T*x, T* y, size_t nsamp, T prevx, T dt) {
        y[0] = (x[0] - prevx)/dt; 
        for (size_t i=1; i<nsamp; i++) {
            y[i] = (x[i] - x[i-1])/dt; 
            prevx = x[i];
        }
    }
}

template <class T>
class wffilter {
  public:
    wffilter();
    ~wffilter();    

    virtual void filter(int npts, T* in, T* out) = 0;
    virtual void filter(int npts, std::vector<T>* in, std::vector<T>* out) = 0; 
};

/** Integration routine designed for continuous recursive computation allowing history values to 
 * be passed, also allowing for simultaneous application of a single pole high pass filter
 * @param vecin vector of input values
 * @param in_nsamp number of samples in the vector to be integrated 
 * @param vecout vector of output values, modified by this function 
 * @param preva previous last value in the integrated array
 * @param prevb previous last value in the input array 
 * @param period delta for the input array samples
 * @param coef coefficient (0 to 1) for applying a filter during integration, defaults to -1 which
 * disables the filter. The coefficient equivalent in cut-off frequency depends on sampling rate 
 * (period).
 */
template <class type> 
void integrate(std::vector<type>* vecin, unsigned int in_nsamp, std::vector<type>* vecout, float preva, float prevb, float period, float coef = -1) {
    if(isnan(preva)) {
        vecout->at(0) = 0.0;
    }
    else {
        if (coef < 0) {
            vecout->at(0) = preva + (vecin->at(0) + prevb) / 2.0; // integration only
        }
        else {
            vecout->at(0) = (1.0 + coef) * (vecin->at(0) + prevb) / 4.0 + (coef * preva);
        }
    }
  
    for(unsigned int i=1;i<in_nsamp;i++) {
        if (coef < 0) {
            vecout->at(i) = vecout->at(i-1) + (vecin->at(i) + vecin->at(i-1)) / 2.0; // integration only 
        }
        else {
            vecout->at(i) = (1.0 + coef) * (vecin->at(i) + vecin->at(i-1)) / 4.0 + (coef * vecout->at(i-1));
        }
    }

    for(unsigned int i=0;i<in_nsamp;i++) {
        vecout->at(i) = vecout->at(i)*period;
    }
 
}

/** Integration routine designed for continuous recursive computation allowing history values to 
 * be passed, also allowing for simultaneous application of a single pole high pass filter
 * @param vecin array of input values
 * @param in_nsamp number of samples in the array to be integrated 
 * @param vecout array of output values, modified by this function 
 * @param preva previous last value in the integrated array
 * @param prevb previous last value in the input array 
 * @param period delta for the input array samples
 * @param coef coefficient (0 to 1) for applying a filter during integration, defaults to -1 which
 * disables the filter. The coefficient equivalent in cut-off frequency depends on sampling rate 
 * (period).
 */
template <class type> 
void integrate(type* vecin, unsigned int in_nsamp, type* vecout, float preva, float prevb, float period, float coef = -1) {
    if(isnan(preva)) {
        vecout[0] = 0.0;
    }
    else {
        if (coef < 0) {
            vecout[0] = preva + (vecin[0] + prevb) / 2.0; // integration only
        }
        else {
            vecout[0] = (1.0 + coef) * (vecin[0] + prevb) / 4.0 + (coef * preva);
        }
    }
  
    for(int i=1;i<in_nsamp;i++) {
        if (coef < 0) {
            vecout[i] = vecout[i-1] + (vecin[i] + vecin[i-1]) / 2.0; // integration only 
        }
        else {
            vecout[i] = (1.0 + coef) * (vecin[i] + vecin[i-1]) / 4.0 + (coef * vecout[i-1]);
        }
    }

    for(int i=0;i<in_nsamp;i++) {
        vecout[i] = vecout[i]*period;
    }
 
}

/** Differentiation routine designed for continuous recursive computation allowing history values 
 * to be passed
 * @param vecin vector of input values
 * @param in_nsamp number of samples in the vector to be integrated 
 * @param vecout vector of output values, modified by this function 
 * @param preva previous last value in the differentiated array
 * @param prevb previous last value in the input array 
 * @param period delta for the input array samples
 */
template <class type>
void differentiate(std::vector<type>* vecin, unsigned int in_nsamp, std::vector<type>* vecout, float preva, float prevb, float period) {
    if (isnan(prevb) || isnan(preva)) {
        vecout->at(0) = 0.0;
    }
    else {
        vecout->at(0) = (vecin->at(0) - prevb) / period;
    }
    for (unsigned int i=1; i<in_nsamp; i++) {
        vecout->at(i) = (vecin->at(i) - vecin->at(i-1)) / period;
    }
}

/** Differentiation routine designed for continuous recursive computation allowing history values 
 * to be passed
 * @param vecin array of input values
 * @param in_nsamp number of samples in the array to be integrated 
 * @param vecout array of output values, modified by this function 
 * @param preva previous last value in the differentiated array
 * @param prevb previous last value in the input array 
 * @param period delta for the input array samples
 */
template <class type>
void differentiate(type* vecin, unsigned int in_nsamp, type* vecout, float preva, float prevb, float period) {
    if (isnan(prevb) || isnan(preva)) {
        vecout[0] = 0.0;
    }
    else {
        vecout[0] = (vecin[0] - prevb) / period;
    }
    for (int i=1; i<in_nsamp; i++) {
        vecout[i] = (vecin[i] - vecin[i-1]) / period;
    }
}

/** enumeration of filter types LP: lowpass, HP: highpass, BP: bandpass, UNKNOW_FILT */
enum FiltType {LP, HP, BP, UNKNOWN_FILT};

/** @struct FiltParams
 * @brief Holds filter parameters such as filter type, order, cut-off frequencies, poles and gains
 * */
struct FiltParams {
    int filttype; /**< Filter type from FiltType enumeration */
    int order; /**< Filter order (number of poles) */
    double lp_cutoff_frequency; /**< Cut-off frequency for low-pass filter */
    double hp_cutoff_frequency; /**< Cut-off frequency for high-pass filter */
    complex lp_poles[BUTTER_MAX_ORDER]; /**< array of complex values of poles for low-pass filter */
    complex hp_poles[BUTTER_MAX_ORDER]; /**< array of complex values of poles for high-pass filter */
    double lp_gain; /**< gain for low-pass filter */
    double hp_gain; /**< gain for high-pass filter */
};

/** Apply highpass Butterworth filter. Butterworth filter using the C version from Jim Fowler 
 * (also used in PQL), IIR cascaded biquad
 * @param in_nsamp number of samples in input array
 * @param in input vector
 * @param out output (filtered) vector, modified by this function
 * @param f1 array of history values
 * @param f2 array of history values
 * @param fltp filter parameters (poles, gain etc.)
 * */
template <class type>
void hpbuttfilt(unsigned int in_nsamp, vector<type>* in, vector<type>* out, double* f1, double* f2, FiltParams* fltp) {
    double *data;
    data = new double [in_nsamp];
    double a1, a2, b1, b2;

    for (unsigned int i=0; i<in_nsamp; i++) {
        data[i] = (double)in->at(i);
    }

    // Get first set of second order filter coefficients from each pair of poles
    for (int i=0; i<fltp->order; i+=2) {
        a1 = -2. * fltp->hp_poles[i].real;
        a2 = fltp->hp_poles[i].real * fltp->hp_poles[i].real + fltp->hp_poles[i].imag * fltp->hp_poles[i].imag;
        b1 = -2.;
        b2 = 1.;
        filt(a1, a2, b1, b2, in_nsamp, data, data, &f1[i], &f2[i]);
    }

    // apply gain
    for (unsigned int i=0 ; i<in_nsamp; i++) {
        out->at(i) = (type)fltp->hp_gain*data[i];
    }

    delete [] data;
}

/** Apply lowpass Butterworth filter. Butterworth filter using the C version from Jim Fowler 
 * (also used in PQL), IIR cascaded biquad
 * @param in_nsamp number of samples in input array
 * @param in input vector
 * @param out output (filtered) vector, modified by this function
 * @param f1 array of history values
 * @param f2 array of history values
 * @param fltp filter parameters (poles, gain etc.)
 * */
template <class type>
void lpbuttfilt(unsigned int in_nsamp, vector<type>* in, vector<type>* out, double* f1, double* f2, FiltParams* fltp) {
    double *data;
    data = new double [in_nsamp];
    double a1, a2, b1, b2;

    for (unsigned int i=0; i<in_nsamp; i++) {
        data[i] = (double)in->at(i);
    }

    // Get first set of second order filter coefficients from each pair of poles
    for (int i=0; i<fltp->order; i+=2) {
        a1 = -2. * fltp->lp_poles[i].real;
        a2 = fltp->lp_poles[i].real * fltp->lp_poles[i].real + fltp->lp_poles[i].imag * fltp->lp_poles[i].imag;
        b1 = 2.;
        b2 = 1.;
        filt(a1, a2, b1, b2, in_nsamp, data, data, &f1[i], &f2[i]);
    }

    // apply gain
    for (unsigned int i=0 ; i<in_nsamp; i++) {
        out->at(i) = (type)fltp->lp_gain*data[i];
    }

    delete [] data;
}

#endif

