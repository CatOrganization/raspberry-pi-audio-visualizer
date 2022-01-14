#include "visualization.hpp"
#include "filter.h"

#include <iostream>

using namespace std;

TripleWaveLineVisualization::TripleWaveLineVisualization(int screen_width, int screen_height, int audio_buffer_size, int audio_sample_rate) :
    VisualizationClass(screen_width, screen_height, audio_buffer_size, audio_sample_rate)
{
    this->base_filtered_audio_frames = new double[audio_buffer_size];
    this->treble_filtered_audio_frames = new double[audio_buffer_size];

    this->downsample_size = min(screen_width, audio_buffer_size);
    this->downsample_buf = new double[this->downsample_size];

    this->base_line = new Vector2[this->downsample_size];
    this->midrange_line = new Vector2[this->downsample_size];
    this->treble_line = new Vector2[this->downsample_size];
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
    float horizontal_scale = (float) this->screen_width / this->audio_buffer_size;

    // WTF is happening with this low pass filter??????????????????????????
    apply_linear_filter(LowPassBassFilter, audio, &this->base_filtered_audio_frames, this->audio_buffer_size);
    apply_linear_filter(HighPassTrebleFilter, audio, &this->treble_filtered_audio_frames, this->audio_buffer_size);

    cout << audio[100] << " wat " << this->base_filtered_audio_frames[100] << endl;
    // Do the base
    downsample_audio(audio, this->audio_buffer_size, this->downsample_buf, this->downsample_size);
    calculate_sound_wave_line_points(this->downsample_buf, this->base_line, this->downsample_size, this->screen_width, 800, 100.0f);



    // Do the treble
    downsample_audio(this->treble_filtered_audio_frames, this->audio_buffer_size, this->downsample_buf, this->downsample_size);
    calculate_sound_wave_line_points(this->downsample_buf, this->treble_line, this->downsample_size, this->screen_width, this->screen_height / 12, this->screen_height / 6.0f);
}

void TripleWaveLineVisualization::draw(bool verbose)
{
    static int counter = 0;
    counter++;

    if (counter % 10 == 0) ClearBackground(BLACK);
//    ClearBackground(BLACK);
    DrawLineStrip(this->base_line, this->downsample_size, (Color) {200, 50, 50, 255});
    DrawLineStrip(this->treble_line, this->downsample_size, (Color) {200, 50, 50, 255});

    DrawRectangle(this->base_line[100].x, this->base_line[100].y, 100, 100, RED);
    if (counter % 10 == 0) {
        cout << this->base_line[100].x << "," << this->base_line[100].y << endl;
    }
}
