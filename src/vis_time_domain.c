#include <stdio.h>
#include <stdlib.h>

#include "visualization.h"
#include "linked_list.h"
#include "effects.h"
#include "filter.h"

void vis_time_domain_init();
void vis_time_domain_update(double *audio_frames);
void vis_time_domain_draw(bool verbose);
void vis_time_domain_clean_up();

// Keeps track of the current X position when looping over the linked list of amplitudes to draw
int vis_time_domain_current_x;

Color vis_time_domain_base_color;
int vis_time_domain_samples_per_frame;
LinkedList vis_time_domain_amplitudes;

Visualization NewTimeDomainVis()
{
    Visualization vis;
    vis.name = "time domain";
    vis.init = vis_time_domain_init;
    vis.update = vis_time_domain_update;
    vis.draw = vis_time_domain_draw;
    vis.clean_up = vis_time_domain_clean_up;

    return vis;
}

void vis_time_domain_init()
{
    vis_time_domain_base_color = SKYBLUE;
    vis_time_domain_samples_per_frame = 10; // 10 new samples added per frame
    vis_time_domain_amplitudes.head = NULL;
    vis_time_domain_amplitudes.size = 0;
}

void vis_time_domain_update(double *audio_frames)
{
    int inc = vis_audio_buffer_frames / vis_time_domain_samples_per_frame;
    if (inc < 1) inc = 1;
    
    for (int n = 0; n < vis_audio_buffer_frames; n += inc)
    {
        double *d = malloc(sizeof(double));
        *d = audio_frames[n];
        linked_list_add(&vis_time_domain_amplitudes, d);
    }

    if (IsKeyDown((char) 'M'))
    {
        vis_time_domain_samples_per_frame++;
        if (vis_time_domain_samples_per_frame > vis_screen_width)
        {
            vis_time_domain_samples_per_frame = vis_screen_width;
        }
    }

    if (IsKeyDown((char) 'L'))
    {
        vis_time_domain_samples_per_frame--;
        if (vis_time_domain_samples_per_frame < 1)
        {
            vis_time_domain_samples_per_frame = 1;
        }
    }
}

int draw_amplitude(void *data)
{
    if (vis_time_domain_current_x < 0)
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
    Color line_color = scale_color(vis_time_domain_base_color, color_scale);

    DrawLine(vis_time_domain_current_x, half_screen_height + half_line_height, vis_time_domain_current_x, half_screen_height - half_line_height, line_color);

    vis_time_domain_current_x--;
    return true;
}

void vis_time_domain_draw(bool verbose)
{
    ClearBackground(BLACK);

    vis_time_domain_current_x = vis_screen_width - 1;
    linked_list_for_each(&vis_time_domain_amplitudes, &draw_amplitude);

    if (verbose)
    {
        char str[128];
        sprintf(str, "size: %d\nsamples per frame: %d\n[m]: more, [l]: less", vis_time_domain_amplitudes.size, vis_time_domain_samples_per_frame);
        DrawText(str, 10, 500, 20, RAYWHITE);
    }
}


int vis_time_domain_linked_list_cleanup_action(void *data)
{
    free(data);
    return false; // remove all items
}

void vis_time_domain_clean_up()
{
    linked_list_for_each(&vis_time_domain_amplitudes, &vis_time_domain_linked_list_cleanup_action);
}
