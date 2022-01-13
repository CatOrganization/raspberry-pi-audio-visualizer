#include "visualization.hpp"
#include "filter.h"

TripleWaveLineVisualization::TripleWaveLineVisualization(int screen_width, int screen_height, int audio_buffer_size, int audio_sample_rate) :
    VisualizationClass(screen_width, screen_height, audio_buffer_size, audio_sample_rate)
{
    this->base_filtered_audio_frames = new double[audio_buffer_length];
    this->treble_filtered_audio_frames = new double[audio_buffer_length];

    int line_size = min(screen_width, audio_buffer_size);
    this->downsample_buf

    this->base_line = new Vector2[line_size];
    this->midrange_line = new Vector2[line_size];
    this->treble_line = new Vector2[line_size];
}

TripleWaveLineVisualization::~TripleWaveLineVisualization()
{
    delete this->base_filtered_audio_frames;
    delete this->treble_filtered_audio_frames;

    delete this->base_line;
    delete this->midrange_line;
    delete this->treble_line;
}

void TripleWaveLineVisualization::update(double *audio)
{
    apply_linear_filter(LowPassBassFilter, audio, &this->base_filtered_audio_frames, this->audio_buffer_size);
    apply_linear_filter(HighPassTrebleFilter, audio, &this->treble_filtered_audio_frames, this->audio_buffer_size);


}

void TripleWaveLineVisualization::draw(bool verbose)
{

}
