#include <stdlib.h>

#include "visualization.h"
#include "effects.h"
#include "filter.h"

static void init();
static void update(double *audio_frames);
static void draw(bool verbose);
static void clean_up();


static Vector2 *sound_wave_line_points;
static int line_thickness;
static Color background_color;
static Color line_color;

Visualization NewSoundWaveVis()
{
    Visualization vis;
    vis.name = "sound wave";
    vis.init = init;
    vis.update = update;
    vis.draw = draw;
    vis.clean_up = clean_up;

    return vis;
}

static void init()
{
    sound_wave_line_points = malloc(sizeof(Vector2) * vis_audio_buffer_samples);
}

static void update(double *audio_frames)
{
    // Calculate sound wave line points and line thickness
    float horizontal_scale = (float) vis_screen_width / vis_audio_buffer_samples;
    int center_y = vis_screen_height / 2;
    double max_y = -1;
    for (int n = 0; n < vis_audio_buffer_samples; n++)
    {
        sound_wave_line_points[n].x = n * horizontal_scale;
        sound_wave_line_points[n].y = center_y + (audio_frames[n] * vis_screen_height);

        if (absf(audio_frames[n]) > max_y)
        {
            max_y = absf(audio_frames[n]);
        }
    }

    line_thickness = max_y * 50;
    if (line_thickness < 1) line_thickness = 1;

    background_color = scale_color(MAGENTA, max_y);
    line_color = (Color) { 255 - background_color.r, 0, 255 - background_color.b, 255 };
}

static void draw(bool verbose)
{
    ClearBackground(background_color);

    for (int n = 0; n < vis_audio_buffer_samples - 1; n++)
    {
        DrawLineEx(sound_wave_line_points[n], sound_wave_line_points[n+1], line_thickness, line_color);
    }
}

static void clean_up()
{
    free(sound_wave_line_points);
}
