#include <math.h>
#include <stdlib.h>

#include "kiss_fft.h"
#include "visualization.h"
#include "effects.h"
#include "filter.h"

static void init();
static void update(double *audio_frames);
static void draw(bool verbose);
static void clean_up();

static kiss_fft_cfg fft_cfg;
static kiss_fft_cpx *fft_in, *fft_out;
static double *magnitudes;
static Vector2 *wave_points;

static double hz_per_step;

Visualization NewFrequencyDomainVis()
{
    Visualization vis;
    vis.name = "frequency domain";
    vis.init = init;
    vis.update = update;
    vis.draw = draw;
    vis.clean_up = clean_up;

    return vis;
}

static void init()
{
    fft_cfg = kiss_fft_alloc(vis_audio_buffer_frames, false, NULL, NULL);
    fft_in = malloc(sizeof(kiss_fft_cpx) * vis_audio_buffer_frames);
    fft_out = malloc(sizeof(kiss_fft_cpx) * vis_audio_buffer_frames);
    magnitudes = malloc(sizeof(double) * (vis_audio_buffer_frames / 2));
    wave_points = malloc(sizeof(Vector2) * vis_audio_buffer_frames);
    
    hz_per_step = vis_audio_sample_rate / (vis_audio_buffer_frames / 2.0);
    fprintf(stdout, "sample_rate: %d; hz_per_step: %f\n", vis_audio_sample_rate, hz_per_step);
}

static void update(double *audio_frames)
{
    // Populate input array
    for (int n = 0; n < vis_audio_buffer_frames; n++)
    {
        fft_in[n].r = audio_frames[n];
        fft_in[n].i = 0;
    }

    kiss_fft(fft_cfg, fft_in, fft_out);

    for (int n = 0; n < vis_audio_buffer_frames / 2; n++)
    {
        magnitudes[n] = 2 * sqrt((fft_out[n].r * fft_out[n].r) + (fft_out[n].i * fft_out[n].i));
    }

    // Regular sound wave stuff:
    int center_y = vis_screen_height / 2;
    for (int n = 0; n < vis_audio_buffer_frames; n++)
    {
        wave_points[n].x = ((float) n) / vis_audio_buffer_frames * vis_screen_width;
        wave_points[n].y = center_y - 50 + (audio_frames[n] * vis_screen_height);
    }
}

static void draw(bool verbose)
{
    ClearBackground(BLACK);

    for (int n = 0; n < vis_audio_buffer_frames - 1; n++)
    {
        DrawLineEx(wave_points[n], wave_points[n+1], 2, GOLD);
    }

    int num_magnitudes = 400; //vis_audio_buffer_frames / 2;
    int bar_width = 3; //vis_screen_width / num_magnitudes - 1;
    for (int n = 1; n < num_magnitudes - 1; n++)
    {
        int magnitude = (magnitudes[n-1] + magnitudes[n]*2 + magnitudes[n+1]) / 2;// + magnitudes[n+1]) / 2;
        DrawRectangle(n * 4, vis_screen_height - magnitude - 100, bar_width, magnitude + 100, GOLD);
    }
}

static void clean_up()
{
    free(fft_in);
    free(fft_out);
    free(magnitudes);
}