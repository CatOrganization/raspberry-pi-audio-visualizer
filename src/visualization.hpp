#ifndef VISUALIZATION_HPP
#define VISUALIZATION_HPP

#include "raylib.h"

class VisualizationClass {
    public:
        VisualizationClass(int screen_width, int screen_height, int audio_buffer_size, int audio_sample_rate) :
            screen_width{screen_width},
            screen_height{screen_height},
            audio_buffer_size{audio_buffer_size},
            audio_sample_rate{audio_sample_rate} {}

        virtual void update(double *audio) = 0;
        virtual void draw(bool verbose) = 0;

    protected:
        int screen_width;
        int screen_height;
        int audio_buffer_size;
        int audio_sample_rate;


        // TODO these are more like helper functions

        // downsample_audio takes audio and downsamples it evenly into output.
        // This function assumes audio_size > output_size
        void downsample_audio(double *audio, int audio_size, double *output, int output_size);

        // calculate_sound_wave_line_points expects audio and output to be of the same length.
        void calculate_sound_wave_line_points(double *audio, Vector2 *output, int audio_size, int baseline_y, float scale);
};

void VisualizationClass::downsample_audio(double *audio, int audio_size, double *output, int output_size)
{
    for (int n = 0; n < output_size; n++)
    {
        int index = n * audio_size / output_size;
        output[n] = audio[index];
    }
}

void VisualizationClass::calculate_sound_wave_line_points(double *audio, Vector2 *output, int audio_size, int baseline_y, float scale)
{
    float horizontal_scale = (float) this->screen_height / audio_size;
    for (int n = 0; n < this->audio_buffer_size; n++)
    {
        output[n].x = n * horizontal_scale;
        output[n].y = baseline_y + (audio[n] * scale);
    }

}

class TripleWaveLineVisualization : public VisualizationClass {
    public:
        TripleWaveLineVisualization(int screen_width, int screen_height, int audio_buffer_length, int audio_sample_rate);
        ~TripleWaveLineVisualization();

        void update(double *audio);
        void draw(bool verbose);

    private:
        double *base_filtered_audio_frames;
        double *treble_filtered_audio_frames;
        double *downsample_buf;

        Vector2 *base_line;
        Vector2 *midrange_line;
        Vector2 *treble_line;

};

#endif
