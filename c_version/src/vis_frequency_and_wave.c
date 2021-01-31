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

Visualization NewFrequencyAndWaveVis()
{
    Visualization vis;
    vis.name = "frequency + wave";
    vis.init = init;
    vis.update = update;
    vis.draw = draw;
    vis.clean_up = clean_up;

    return vis;
}

static void init()
{
    fft_cfg = kiss_fft_alloc(vis_audio_buffer_samples, false, NULL, NULL);
    fft_in = malloc(sizeof(kiss_fft_cpx) * vis_audio_buffer_samples);
    fft_out = malloc(sizeof(kiss_fft_cpx) * vis_audio_buffer_samples);
    magnitudes = malloc(sizeof(double) * (vis_audio_buffer_samples / 2));
    wave_points = malloc(sizeof(Vector2) * vis_audio_buffer_samples);
    
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

    // Regular sound wave stuff:
    int center_y = vis_screen_height / 2;
    for (int n = 0; n < vis_audio_buffer_samples; n++)
    {
        wave_points[n].x = ((float) n) / vis_audio_buffer_samples * vis_screen_width;
        wave_points[n].y = center_y + (audio_frames[n] * vis_screen_height);
    }
}

static void draw(bool verbose)
{
    ClearBackground(BLACK);

    for (int n = 0; n < vis_audio_buffer_samples - 1; n++)
    {
        DrawLineEx(wave_points[n], wave_points[n+1], 2, GOLD);
    }

    int num_magnitudes = 400;
    int bar_width = 3;
    for (int n = 1; n < num_magnitudes - 1; n++)
    {    	
    	  int mag_index = n;
		  if (n > num_magnitudes / 2)
		  {
				mag_index = num_magnitudes - n;		  
		  }		  
		      	        
		  int magnitude = (magnitudes[mag_index-1] + magnitudes[mag_index]*2 + magnitudes[mag_index+1]) / 2;// + magnitudes[n+1]) / 2;
        DrawRectangle(n * 4, vis_screen_height - magnitude - 50, bar_width, magnitude + 100, GOLD);
        DrawRectangle(n * 4, 0, bar_width, magnitude + 50, GOLD);
    }
}

static void clean_up()
{
    free(fft_in);
    free(fft_out);
    free(magnitudes);
    free(wave_points);
}