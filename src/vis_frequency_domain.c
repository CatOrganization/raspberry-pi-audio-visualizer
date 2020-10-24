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

static int size = 800;
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
    fft_cfg = kiss_fft_alloc(size, false, NULL, NULL);
    fft_in = malloc(sizeof(kiss_fft_cpx) * size);
    fft_out = malloc(sizeof(kiss_fft_cpx) * size);
    magnitudes = malloc(sizeof(double) * (size / 2));
    
    hz_per_step = vis_audio_sample_rate / (size / 2.0);
    fprintf(stdout, "sample_rate: %d; hz_per_step: %f\n", vis_audio_sample_rate, hz_per_step);
}

static void update(double *audio_frames)
{
    // Populate input array
    for (int n = 0; n < size; n++)
    {
        fft_in[n].r = audio_frames[n] * M_PI; //sin(2 * M_PI * 4 * n / size); //audio_frames[n];
        fft_in[n].i = 0;
    }

    kiss_fft(fft_cfg, fft_in, fft_out);

    for (int n = 0; n < size / 2; n++)
    {
        magnitudes[n] = sqrt((fft_out[n].r * fft_out[n].r) + (fft_out[n].i * fft_out[n].i));
    }
}

static void draw(bool verbose)
{
    ClearBackground(BLACK);

    Color color = (Color) {0, 0, 0, 255};
    Color blue = (Color) {0, 0, 255, 255};
    int horizontal_scale = vis_screen_width / (size / 4);
    for (int n = 0; n < size / 2; n += 2)
    {
        int magnitude = (magnitudes[n] + magnitudes[n+1]) * 10;
        if (magnitude > vis_screen_height) magnitude = vis_screen_height;
        double scale = magnitude / (double) vis_screen_height;
        scale = pow(scale - 1, 3) + 1;

        color.r = scale * 255;
        color.b = 1 - color.r;

        DrawRectangleGradientV((n/2) * horizontal_scale, vis_screen_height - magnitude, horizontal_scale, magnitude, color, blue);
    }
}

static void clean_up()
{
    free(fft_in);
    free(fft_out);
    free(magnitudes);
}