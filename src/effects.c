#include <math.h>
#include <stdlib.h>
#include <string.h>

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

WaveLine init_wave_line(Shader wave_line_shader, const char *uniform_prefix, int screen_width, int screen_height)
{
    WaveLine wave_line;
    wave_line.shader = wave_line_shader;
    
    char str[30] = "";

    strcpy(str, uniform_prefix);
    strcat(str, "y");
    wave_line.uniform_y = GetShaderLocation(wave_line_shader, str);

    strcpy(str, uniform_prefix);
    strcat(str, "intensity");
    wave_line.uniform_intensity = GetShaderLocation(wave_line_shader, str);

    strcpy(str, uniform_prefix);
    strcat(str, "color");
    wave_line.uniform_color = GetShaderLocation(wave_line_shader, str);

    wave_line.screen_width = screen_width;
    wave_line.screen_height = screen_height;

    return wave_line;
}

void set_wave_line_intensity(WaveLine *wave_line, float intensity)
{
    SetShaderValue(wave_line->shader, wave_line->uniform_intensity, &intensity, UNIFORM_FLOAT);
}

void set_wave_line_color(WaveLine *wave_line, Color color)
{
    Vector4 normalized_color = ColorNormalize(color);
    SetShaderValue(wave_line->shader, wave_line->uniform_color, &normalized_color.x, UNIFORM_VEC4);
}

void draw_wave_line(WaveLine *wave_line, float y)
{
    SetShaderValue(wave_line->shader, wave_line->uniform_y, &y, UNIFORM_FLOAT);

    BeginShaderMode(wave_line->shader);
        DrawRectangle(0, 0, wave_line->screen_width, wave_line->screen_height, RAYWHITE);
    EndShaderMode();
}

