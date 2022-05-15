#ifndef RASPBERRY_PI_AUDIO_VISUALIZER_AUDIO_SOURCE_HPP
#define RASPBERRY_PI_AUDIO_VISUALIZER_AUDIO_SOURCE_HPP

#include "alsa/asoundlib.h"

#include <atomic>
#include <pthread.h>
#include <string>
#include <sndfile.h>

#include <unistd.h>
#include <iostream>

using namespace std;

class AudioSource {
    public:
        AudioSource(int buffer_size, int frame_rate);
        ~AudioSource();

        void *run_read_loop_in_thread() { this->run_read_loop(); return NULL; }

        // run_read_loop continuously reads audio and updates internal state.
        // You should run this in a separate thread, then call copy_audio_data(...) whenever you're ready for audio_data.
        virtual void run_read_loop() = 0;

        // copy_audio_data puts this AudioSource's most recent audio data in dst.
        // dst MUST be at least of size `buffer_size`.
        void copy_audio_data(double *dst);

        int get_buffer_size() const { return buffer_size; }
        virtual int get_audio_sample_rate() = 0;

        void close() { this->atomic_done = true; }

    protected:
        bool done() { return this->atomic_done.load(); }

        int buffer_size;
        int frame_rate;

        pthread_mutex_t audio_frames_buffer_lock;
        double *audio_frames_buffer;

    private:
        atomic<bool> atomic_done;
};

// Get an audio source given an arg from a user.
// If this arg ends in `.wav` this will return a WAVAudioSource,
// Otherwise it assumes the arg is a hardware audio source (run `arecord -l` for a list of hw sources)
// and returns an ALSAAudioSource.
AudioSource *get_audio_source(int buffer_size, int frame_rate, char *arg);

class WAVAudioSource : public AudioSource {
    public:
        WAVAudioSource(int buffer_size, int frame_frate, string filename);
        ~WAVAudioSource();

        void run_read_loop();

        int get_audio_sample_rate() { return this->samplerate; }
    private:
        int samplerate;
        int num_audio_frames;
        short *audio_data;
        
        snd_pcm_t *output_handle;
};


class ALSAAudioSource : public AudioSource {
    public:
        ALSAAudioSource(int buffer_size, int frame_rate, char *hw_src);

        void run_read_loop();
        int get_audio_sample_rate() { return this->samplerate; }
    private:
        unsigned int samplerate;
        int samples_per_frame;
        
        snd_pcm_t *input_handle;
};

#endif //RASPBERRY_PI_AUDIO_VISUALIZER_AUDIO_SOURCE_HPP
