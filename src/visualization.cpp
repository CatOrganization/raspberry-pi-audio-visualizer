#include "visualization.hpp"

// downsample_audio takes audio and downsamples it evenly into output.
// This function assumes audio_size >= output_size
void downsample_audio(double *audio, int audio_size, double *output, int output_size)
{
    for (int n = 0; n < output_size; n++)
    {
        int index = n * audio_size / output_size;
        output[n] = audio[index];
    }
}

void calculate_sound_wave_line_points(double *audio, Vector2 *output, int audio_size, int screen_width, int baseline_y, float scale)
{
    float horizontal_scale = (float) screen_width / audio_size;
    for (int n = 0; n < audio_size; n++)
    {
        output[n].x = n * horizontal_scale;
        output[n].y = baseline_y + (audio[n] * scale);
        //output[n].y = baseline_y + ((((float)n / audio_size * 2) - 1) * scale);
    }

}
