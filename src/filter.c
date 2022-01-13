#include "filter.h"

// Low pass bass filter that filters out frequencies > 500Hz
// Coefficients calculated via SciPy: scipy.signal.butter(4, 500, bytpe='lowpass', fs=44100)
double lowPassBassFilterNumeratorCoefficients[] = {1.46901845e-6, 5.87607379e-6, 8.81411069e-6, 5.87607379e-6, 1.46901845e-6};
double lowPassBassFilterDenominatorCoefficients[] = {-3.81386538, 5.45872379, -3.47494261, 0.83010771};
const LinearFilter LowPassBassFilter = {
        4, // 4th order linear filter
        lowPassBassFilterNumeratorCoefficients,
        lowPassBassFilterDenominatorCoefficients
};

// High pass treble filter that filters our frequencies < 2000Hz
// Coefficients calculated via SciPy: scipy.signal.butter(4, 2000, btype='highpass', fs=44100)
double highPassTrebleFilterNumeratorCoefficients[] = {0.68811799, -2.75247196, 4.12870794, -2.75247196, 0.68811799};
double highPassTrebleFilterDenominatorCoefficients[] = {-3.25656931, 4.03376839, -2.24604368, 0.47350645};
const LinearFilter HighPassTrebleFilter = {
    4, // 4th order linear filter
    highPassTrebleFilterNumeratorCoefficients,
    highPassTrebleFilterDenominatorCoefficients
};

double absf(double d)
{
    if (d < 0) return -d;
    return d;
}

double apply_linear_filter(LinearFilter filter, double *input, double **output, int size)
{
    double input_influence = 0;
    double output_influence = 0;
    double max = 0;

    // We have to start at i = filter.order so we have enough look back buffer
    for (int i = filter.order; i < size; i++)
    {
        input_influence = 0;
        output_influence = 0;

        // Calculate the input_influence value
        for (int j = 0; j <= filter.order; j++)
        {
            input_influence += filter.b[j] * input[i - j];
        }

        // Calculate the output_influence value
        for (int j = 0; j < filter.order; j++)
        {
            output_influence += filter.a[j] * (*output)[i - j - 1];
        }

        (*output)[i] = input_influence - output_influence;

        if (absf((*output)[i]) > max)
        {
            max = absf((*output)[i]);
        }
    }

    return max;
}
