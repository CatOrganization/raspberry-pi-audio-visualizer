#ifndef EFFECTS_H
#define EFFECTS_H

#include "raylib.h"

#define NUM_FIREWORK_PARTICLES 20
#define FIREWORK_DURATION 20

// Gets a random color at the given saturation level
Color get_random_color(float saturation);

// Scales the color by the given scale factor
Color scale_color(Color color, float scale_factor);

// Blends from color a to b. ie blend_colors(a, b, 0.0) = a; blend_colors(a, b, 1.0) = b
Color blend_colors(Color a, Color b, float blend_percentage);

// Gets a random number between the given min and max
double get_random_number(double min, double max);

typedef struct Firework {
    Vector2 particles[NUM_FIREWORK_PARTICLES];
    Vector2 particle_velocities[NUM_FIREWORK_PARTICLES];
    
    Color color;
    int steps_remaining;
} Firework;

// Creates a new firework at the given location with the given parameters 
Firework *new_firework(int center_x, int center_y, Color base_color, double duration_coefficient);

// Draw the given firework; must be called in the draw phase
void draw_firework(Firework *firework);

// Update the given firework. Returns false if the firework is finished updating and should be removed
bool update_firework(Firework *firework);

typedef struct WaveLine {
    Shader shader;
    int uniform_y;
    int uniform_intensity;
    int uniform_color;

    int screen_width;
    int screen_height;
} WaveLine;

WaveLine init_wave_line(Shader wave_shader, const char *uniform_prefix, int screen_width, int screen_height);
void set_wave_line_intensity(WaveLine *wave_line, float intensity);
void set_wave_line_color(WaveLine *wave_line, Color color);

void draw_wave_line(WaveLine *wave_line, float y);

#endif
