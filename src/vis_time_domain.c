#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include "visualization.h"
#include "linked_list.h"
#include "effects.h"
#include "filter.h"

static void init();
static void update(double *audio_frames);
static void draw(bool verbose);
static void clean_up();

// Keeps track of the current X position when looping over the linked list of amplitudes to draw
static int current_x;

static Color base_color;
static int samples_per_frame;
static LinkedList amplitudes;

Visualization NewTimeDomainVis()
{
    Visualization vis;
    vis.name = "time domain";
    vis.init = init;
    vis.update = update;
    vis.draw = draw;
    vis.clean_up = clean_up;

    return vis;
}

static void init()
{
    base_color = SKYBLUE;
    samples_per_frame = 10; // 10 new samples added per frame
    amplitudes.head = NULL;
    amplitudes.size = 0;
}

static void update(double *audio_frames)
{
    int data_points_per_sample = vis_audio_buffer_samples / samples_per_frame;
    if (data_points_per_sample < 1) data_points_per_sample = 1;

    double current_max = absf(audio_frames[0]);

    for (int n = 1; n < vis_audio_buffer_samples; n++)
    {
        if (absf(audio_frames[n]) > current_max)
        {
            current_max = absf(audio_frames[n]);
        }

        if (n % data_points_per_sample == 0)
        {
            double *d = malloc(sizeof(double));
            *d = current_max;
            linked_list_add(&amplitudes, d);

            current_max = 0;
        }
    }

    if (IsKeyDown((char) 'M'))
    {
        samples_per_frame++;
        if (samples_per_frame > vis_screen_width)
        {
            samples_per_frame = vis_screen_width;
        }
    }

    if (IsKeyDown((char) 'L'))
    {
        samples_per_frame--;
        if (samples_per_frame < 1)
        {
            samples_per_frame = 1;
        }
    }
}

static int draw_amplitude(void *data)
{
    if (current_x < 0)
    {
        free(data);
        return false; // Remove this item if we've gone beyond the screen
    }

    int half_screen_height = vis_screen_height / 2;
    double amplitude = *((double *) data);
    int half_line_height = half_screen_height * amplitude;
    
    float color_scale = absf((float) amplitude) + 0.2;
    if (color_scale > 1) color_scale = 1;

    // This equation makes small changes more noticable at smaller values
    // ie 0.01 -> 0.02 results in a larger color change than 0.90 -> 0.91
    color_scale = pow(color_scale - 1, 3) + 1;
    Color line_color = scale_color(base_color, color_scale);

    DrawLine(current_x, half_screen_height + half_line_height, current_x, half_screen_height - half_line_height, line_color);

    current_x--;
    return true;
}

static void draw(bool verbose)
{
    ClearBackground(BLACK);

    current_x = vis_screen_width - 1;
    linked_list_for_each(&amplitudes, &draw_amplitude);

    if (verbose)
    {
        char str[128];
        sprintf(str, "size: %d\nsamples per frame: %d\n[m]: more, [l]: less", amplitudes.size, samples_per_frame);
        DrawText(str, 10, 500, 20, RAYWHITE);
    }
}


static int linked_list_cleanup_action(void *data)
{
    free(data);
    return false; // remove all items
}

void clean_up()
{
    linked_list_for_each(&amplitudes, &linked_list_cleanup_action);
}
