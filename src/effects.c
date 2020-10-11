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