#include <math.h>
#include <stdlib.h>

#include "effects.h"

double get_random_number(double min, double max)
{
    double range = max - min;
    return min + (((double) rand() / RAND_MAX) * range);
}

Color get_random_color(float saturation)
{
    float hue = get_random_number(0.0, 360);
    return ColorFromHSV(hue, saturation, 1.0f);
}

Color scale_color(Color color, float scale_factor)
{
    Color output;

    output.r = scale_factor * color.r;
    output.g = scale_factor * color.g;
    output.b = scale_factor * color.b;

    if (output.r < 5) output.r = 0;
    if (output.g < 5) output.g = 0;
    if (output.b < 5) output.b = 0;

    return output;
}
