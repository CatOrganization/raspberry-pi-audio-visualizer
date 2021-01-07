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
static double buckets[15];

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

	 buckets[0] = 0;
	 buckets[1] = 0;
	 buckets[2] = 0;
buckets[3] = 0;
buckets[4] = 0;
buckets[5] = 0;
buckets[6] = 0;
buckets[7] = 0;
buckets[8] = 0;
buckets[9] = 0;
buckets[10] = 0;
buckets[11] = 0;
buckets[12] = 0;
buckets[13] = 0;
    for (int n = 0; n < size / 2; n++)
    {
        magnitudes[n] = sqrt((fft_out[n].r * fft_out[n].r) + (fft_out[n].i * fft_out[n].i));
    	  if (n < 2) 
    	  {
				buckets[0] += magnitudes[n] * 3;
    	  }
    	  else if (n < 4)
    	  {
    	  		buckets[1] = magnitudes[n] * 2;
    	  }
    	  else if (n < 8)
    	  {
				buckets[2] += magnitudes[n] / 2;    	  
    	  }
    	  else if (n < 16)
    	  {
    	  		buckets[3] += magnitudes[n] / 2;
    	  }
    	  else if (n < 32)
    	  {
    	  		buckets[4] += magnitudes[n] / 3;
    	  }
    	  else if (n < 64)
    	  {
    	  		buckets[5] += magnitudes[n] / 4;    	  
		  }
		  else if (n < 96) 
		  {
				buckets[6] += magnitudes[n] / 4;
		  }
		  else if (n < 128)
		  {
		  		buckets[7] += magnitudes[n] / 4;	  
		  }
		  else if (n < 200)
		  {
		  		buckets[8] += magnitudes[n] / 4;			  
		  }
		  else if (n < 250)
		  {
		  		buckets[9] += magnitudes[n] / 4;			  
		  }
		  else if (n < 300)
		  {
		  		buckets[10] += magnitudes[n] / 4;			  
		  }
else if (n < 350)
		  {
		  		buckets[11] += magnitudes[n] / 4;			  
		  }
		  else //if (n < 250)
		  {
		  		buckets[12] += magnitudes[n] / 4;			  
		  }
	 }
}

static void draw(bool verbose)
{
    ClearBackground(BLACK);

    Color color = (Color) {0, 0, 0, 255};
    Color blue = (Color) {0, 0, 255, 255};
   
    for (int n = 0; n < 13; n++)
    {
    	int magnitude = buckets[n] * 10;
        if (magnitude > vis_screen_height) magnitude = vis_screen_height;
        double scale = magnitude / (double) vis_screen_height;
        scale = pow(scale - 1, 3) + 1;

        color.r = scale * 255;
        color.b = 1 - color.r;

			DrawRectangleGradientV(n * 123, vis_screen_height - magnitude, 123, magnitude, color, blue);
	    }   
    /*
int horizontal_scale = vis_screen_width / (size / 4);
    for (int n = 0; n < size / 2; n += 2)
    {
        int magnitude = (magnitudes[n] + magnitudes[n+1]) * 10;
        if (magnitude > vis_screen_height) magnitude = vis_screen_height;
        double scale = magnitude / (double) vis_screen_height;
        scale = pow(scale - 1, 3) + 1;

        color.r = scale * 255;
        color.b = 1 - color.r;

        //DrawRectangleGradientV((n/2) * horizontal_scale, vis_screen_height - magnitude, horizontal_scale, magnitude, color, blue);
    }
    */
   
}

static void clean_up()
{
    free(fft_in);
    free(fft_out);
    free(magnitudes);
}