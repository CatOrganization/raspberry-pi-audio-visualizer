#include <math.h>

#include <stdlib.h>
#include "effects.h"

#define FIREWORK_VELOCITY_MAX 20

double get_random_number(double min, double max)
{
    double range = max - min;
    return min + (((double) rand() / RAND_MAX) * range);
}

Color get_random_color(float saturation)
{
    Vector3 hsv;

    hsv.x = get_random_number(0.0, 360);
    hsv.y = saturation;
    hsv.z = 1.0f;

    return ColorFromHSV(hsv);
}

Firework *new_firework(int x, int y, Color base_color, double duration_coefficient)
{
    Firework *firework = malloc(sizeof(Firework));
    
    for (int i = 0; i < NUM_FIREWORK_PARTICLES; i++)
    {
        firework->particles[i].x = x;
        firework->particles[i].y = y;

        firework->particle_velocities[i].x = get_random_number(-FIREWORK_VELOCITY_MAX, FIREWORK_VELOCITY_MAX);
        firework->particle_velocities[i].y = get_random_number(-FIREWORK_VELOCITY_MAX, FIREWORK_VELOCITY_MAX);
    }

    firework->color = base_color;
    firework->steps_remaining = FIREWORK_DURATION * duration_coefficient;

    return firework;
}

void draw_firework(Firework *firework)
{
    for (int i = 0; i < NUM_FIREWORK_PARTICLES; i++)
    {
        DrawRectangle(firework->particles[i].x, firework->particles[i].y, 3, 3, firework->color);
        //DrawPixel(firework->particles[i].x, firework->particles[i].y, firework->color);
    }
}

bool update_firework(Firework *firework)
{
    for (int i = 0; i < NUM_FIREWORK_PARTICLES; i++)
    {
        // Update position
        firework->particles[i].x += firework->particle_velocities[i].x;
        firework->particles[i].y += firework->particle_velocities[i].y;
        
        // Update velocity
        firework->particle_velocities[i].x *= 0.95f; // "air resistance"
        firework->particle_velocities[i].y += 1; // "Gravity"
    }

    firework->steps_remaining--;
    
    if (firework->steps_remaining > FIREWORK_DURATION)
    {
        firework->color.a = 255;
    } 
    else 
    {
        double duration_fraction = firework->steps_remaining / (double) FIREWORK_DURATION;
        double coefficient = pow(duration_fraction - 1, 3) + 1; // (x-1)^3 + 1
        firework->color.a = (char) (255 * coefficient);
    }

    return firework->steps_remaining > 0;
}
