#include <stdlib.h>

#include "visualization.h"
#include "effects.h"
#include "filter.h"

void vis_sound_wave_init();
void vis_sound_wave_update(double *audio_frames);
void vis_sound_wave_draw(bool verbose);
void vis_sound_wave_clean_up();

typedef struct VisSoundWaveMetadata {
    Vector2 *sound_wave_line_points;
    int line_thickness;
    Color background_color;
    Color line_color;
} VisSoundWaveMetadata;

VisSoundWaveMetadata vis_sound_wave_metadata;

Visualization NewSoundWaveVis()
{
    Visualization vis;
    vis.name = "sound wave";
    vis.init = vis_sound_wave_init;
    vis.update = vis_sound_wave_update;
    vis.draw = vis_sound_wave_draw;
    vis.clean_up = vis_sound_wave_clean_up;

    return vis;
}

void vis_sound_wave_init()
{
    vis_sound_wave_metadata.sound_wave_line_points = malloc(sizeof(Vector2) * vis_audio_buffer_frames);
}

void vis_sound_wave_update(double *audio_frames)
{
    // Calculate sound wave line points and line thickness
    float horizontal_scale = vis_screen_width / vis_audio_buffer_frames;
    int center_y = vis_screen_height / 2;
    double max_y = -1;
    for (int n = 0; n < vis_audio_buffer_frames; n++)
    {
        vis_sound_wave_metadata.sound_wave_line_points[n].x = n * horizontal_scale;
        vis_sound_wave_metadata.sound_wave_line_points[n].y = center_y + (audio_frames[n] * vis_screen_height);

        if (absf(audio_frames[n]) > max_y)
        {
            max_y = absf(audio_frames[n]);
        }
    }

    vis_sound_wave_metadata.line_thickness = max_y * 50;
    if (vis_sound_wave_metadata.line_thickness < 1) vis_sound_wave_metadata.line_thickness = 1;

    vis_sound_wave_metadata.background_color = scale_color(MAGENTA, max_y);
    vis_sound_wave_metadata.line_color = (Color) { 255 - vis_sound_wave_metadata.background_color.r, 0, 255 - vis_sound_wave_metadata.background_color.b, 255 };
}

void vis_sound_wave_draw(bool verbose)
{
    ClearBackground(vis_sound_wave_metadata.background_color);

    for (int n = 0; n < vis_audio_buffer_frames - 1; n++)
    {
        DrawLineEx(vis_sound_wave_metadata.sound_wave_line_points[n], vis_sound_wave_metadata.sound_wave_line_points[n+1], vis_sound_wave_metadata.line_thickness, vis_sound_wave_metadata.line_color);
    }
}

void vis_sound_wave_clean_up()
{
    free(vis_sound_wave_metadata.sound_wave_line_points);
}
