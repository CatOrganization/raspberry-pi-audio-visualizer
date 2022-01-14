#ifndef VISUALIZATION_HPP
#define VISUALIZATION_HPP

#include "raylib.h"
#include "visualization.h"

// downsample_audio takes audio and downsamples it evenly into output.
// This function assumes audio_size >= output_size
void downsample_audio(double *audio, int audio_size, double *output, int output_size);

void calculate_sound_wave_line_points(double *audio, Vector2 *output, int audio_size, int screen_width, int baseline_y, float scale);

class VisualizationClass {
    public:
        VisualizationClass(int screen_width, int screen_height, int audio_buffer_size, int audio_sample_rate) :
            screen_width{screen_width},
            screen_height{screen_height},
            audio_buffer_size{audio_buffer_size},
            audio_sample_rate{audio_sample_rate} {}

        virtual void update(double *audio) = 0;
        virtual void draw(bool verbose) = 0;
        virtual const char *name() = 0;

    protected:
        int screen_width;
        int screen_height;
        int audio_buffer_size;
        int audio_sample_rate;
};

class VisualizationWrapper : public VisualizationClass {
    public:
        VisualizationWrapper(Visualization viz) : viz{viz}, VisualizationClass(0, 0, 0, 0)
        { viz.init(); }

        ~VisualizationWrapper()
        { this->viz.clean_up(); }

        void update(double *audio)
        { this->viz.update(audio); }

        void draw(bool verbose)
        { this->viz.draw(verbose); }

        const char *name()
        { this->viz.name; }

    private:
        Visualization viz;
};


// IDEA: sound wave, but like with a mouse trail thing where it shows fading old ones too ya know?
class FadingLineVisualization : public VisualizationClass {
    public:
        FadingLineVisualization(int screen_width, int screen_height, int audio_buffer_length, int audio_sample_rate);
        ~FadingLineVisualization();

        void update(double *audio);
        void draw(bool verbose);

        const char *name() { return "fading wave line"; }

    private:
        Vector2 **lines;

        int downsample_size;
        double *downsample_buf;

};

class TripleWaveLineVisualization : public VisualizationClass {
    public:
        TripleWaveLineVisualization(int screen_width, int screen_height, int audio_buffer_length, int audio_sample_rate);
        ~TripleWaveLineVisualization();

        void update(double *audio);
        void draw(bool verbose);

        const char *name() { return "triple wave line"; }

    private:
        double *base_filtered_audio_frames;
        double *treble_filtered_audio_frames;

        Vector2 *base_line;
        Vector2 *midrange_line;
        Vector2 *treble_line;

        int downsample_size;
        double *downsample_buf;

};

#endif
