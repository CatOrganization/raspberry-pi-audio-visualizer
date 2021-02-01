#ifndef FILTER_H
#define FILTER_H

// LinearFilter represents a discretized linear filter that can be applied to a stream of audio values
// We assume the filter was discretized with a sampling frequency of 44100Hz
typedef struct LinearFilter 
{
    int order; // The order of this n-th order linear filter (aka size of the 'b' coefficient array)

    double *b; // array of numerator polynomial coefficients for the linear filter (should be of size `order`)
    double *a; // array of denominator polynomial coefficients for the linear filter (should be of size `order - 1`)
} LinearFilter;

// Low pass bass filter that filters out frequencies > 500Hz
// Coefficients calculated via SciPy: scipy.signal.butter(4, 500, bytpe='lowpass', fs=44100)
const LinearFilter LowPassBassFilter; 

// High pass treble filter that filters our frequencies < 2000Hz
// Coefficients calculated via SciPy: scipy.signal.butter(4, 2000, btype='highpass', fs=44100)
const LinearFilter HighPassTrebleFilter;

// absf returns the absolute value of the given double
double absf(double d);

// Applies the given filter to the input and puts the result in output
// Returns the amplitude of the loudest wave in the given sample
double apply_linear_filter(LinearFilter filter, double *input, double **output, int size);

#endif
