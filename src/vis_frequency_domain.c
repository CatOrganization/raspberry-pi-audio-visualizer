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
    magnitudes = malloc(sizeof(double) * size);
}

static void update(double *audio_frames)
{
    static bool did_it = false;
    // Populate input array
    for (int n = 0; n < size; n++)
    {
        fft_in[n].r = audio_frames[n] * M_PI; //sin(2 * M_PI * 4 * n / size); //audio_frames[n];
        fft_in[n].i = 0;
    }

    kiss_fft(fft_cfg, fft_in, fft_out);

    for (int n = 0; n < size; n++)
    {
        magnitudes[n] = sqrt((fft_out[n].r * fft_out[n].r) + (fft_out[n].i * fft_out[n].i));
        if (!did_it)
        {
            fprintf(stdout, "in[%d] = (%f, %f),\tout[%d] = (%f, %f); mag[%d]: %f\n", 
                n, fft_in[n].r, fft_in[n].i,
                n, fft_out[n].r, fft_out[n].i,
                n, magnitudes[n]);
        }
    }

    did_it = true;
}

static void draw(bool verbose)
{
    ClearBackground(BLACK);

    int horizontal_scale = vis_screen_width / (size / 2);
    for (int n = 0; n < size - 1; n++)
    {
        //DrawPixel(n * horizontal_scale, magnitudes[n], RAYWHITE);
        DrawLine(n * horizontal_scale, magnitudes[n] + 300, (n + 1) * horizontal_scale, magnitudes[n+1] + 300, RAYWHITE);
        DrawLine(n * horizontal_scale, fft_in[n].r + 100, (n + 1) * horizontal_scale, fft_in[n+1].r + 100, SKYBLUE);
    }
}

static void clean_up()
{
    free(fft_in);
    free(fft_out);
    free(magnitudes);
}