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
static double hz_per_step;

static int num_lines = 5;

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
    fft_cfg = kiss_fft_alloc(vis_audio_buffer_samples, false, NULL, NULL);
    fft_in = (kiss_fft_cpx*) malloc(sizeof(kiss_fft_cpx) * vis_audio_buffer_samples);
    fft_out = (kiss_fft_cpx*) malloc(sizeof(kiss_fft_cpx) * vis_audio_buffer_samples);
    magnitudes = (double*) malloc(sizeof(double) * (vis_audio_buffer_samples / 2));

    hz_per_step = vis_audio_sample_rate / (vis_audio_buffer_samples / 2.0);
    fprintf(stdout, "sample_rate: %d; hz_per_step: %f\n", vis_audio_sample_rate, hz_per_step);
}

static void update(double *audio_frames)
{
    // Populate input array
    for (int n = 0; n < vis_audio_buffer_samples; n++)
    {
        fft_in[n].r = audio_frames[n];
        fft_in[n].i = 0;
    }

    kiss_fft(fft_cfg, fft_in, fft_out);

    for (int n = 0; n < vis_audio_buffer_samples / 2; n++)
    {
        magnitudes[n] = 2 * sqrt((fft_out[n].r * fft_out[n].r) + (fft_out[n].i * fft_out[n].i));
    }

    if (IsKeyDown((char) 'M'))
    {
        num_lines++;
    }

    if (IsKeyDown((char) 'L'))
    {
        num_lines--;
        if (num_lines < 1) num_lines = 1;
    }
}

static void draw_mirrored_rect(int x, int y, int width, int height, Color color)
{
    DrawRectangle(x, y - height, width, height * 2, color);
}

static int determine_x_coord(float percentage, int num_lines)
{
    float percentage_per_line = 1.0f / num_lines;
    bool should_reverse = false;

    while (percentage > percentage_per_line)
    {
        percentage -= percentage_per_line;
        should_reverse = !should_reverse;
    }

    if (should_reverse)
    {
        return (1.0f - (percentage / percentage_per_line)) * vis_screen_width;
    }
    else
    {
        return (percentage / percentage_per_line) * vis_screen_width;
    }
}

static int determine_y_coord(float percentage)
{
    return 20 + (percentage * (vis_screen_height - 40));
}

static void draw(bool verbose)
{
    ClearBackground(BLACK);

    int num_magnitudes = 20000/hz_per_step; // This means no matter what our sample rate or frame buffer is, we always show the same spectrum (0-20kHz)
    int total_width = vis_screen_width * num_lines;

    int bar_width = total_width / num_magnitudes;
    if (bar_width < 1) bar_width = 1;

    for (int n = 1; n < num_magnitudes - 1; n++)
    {
        double magnitude = (magnitudes[n-1] + magnitudes[n]*2 + magnitudes[n+1]) / 2.0;// + magnitudes[n+1]) / 2;
        int log_magnitude = magnitude == 0 ? 0 : (int) (log((double) magnitude + 1) * 5);

        float percentage = n / (float) num_magnitudes;

        int x = determine_x_coord(percentage, num_lines);
        int y = determine_y_coord(percentage);
        Color color = blend_colors(DARKBLUE, DARKPURPLE, percentage);

        draw_mirrored_rect(x, y, bar_width, log_magnitude + 5, color);
    }

    if (verbose)
    {
        DrawText("m: more lines\nl: less lines", 5, 400, 20, RAYWHITE);
    }
}

static void clean_up()
{
    free(fft_in);
    free(fft_out);
    free(magnitudes);
}
