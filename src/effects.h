#ifndef EFFECTS_H
#define EFFECTS_H

#include "raylib.h"

#define NUM_FIREWORK_PARTICLES 20
#define FIREWORK_DURATION 30

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


#endif
