#ifndef VISUALIZATION_H
#define VISUALIZATION_H

#include "raylib.h"

typedef void (vis_init_func)();
typedef void (vis_update_func)(double *audio_frames);
typedef void (vis_draw_func)(bool verbose);
typedef void (vis_clean_up_func)();

// These value are populated in main.c
int vis_screen_width;
int vis_screen_height;
int vis_audio_buffer_samples;
unsigned int vis_audio_sample_rate;

typedef struct Visualization {
    const char *name;

    vis_init_func *init;
    vis_update_func *update;
    vis_draw_func *draw;
    vis_clean_up_func *clean_up;
} Visualization;

Visualization NewFireworksAndWavesVis();
Visualization NewSoundWaveVis();
Visualization NewDvdLogoVis();
Visualization NewTimeDomainVis();
Visualization NewFrequencyDomainVis();

#endif