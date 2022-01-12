#ifndef RASPBERRY_PI_AUDIO_VISUALIZER_AUDIO_SOURCE_HPP
#define RASPBERRY_PI_AUDIO_VISUALIZER_AUDIO_SOURCE_HPP

#include <atomic>
#include <pthread.h>
#include <string>
#include <sndfile.h>

#include <unistd.h>
#include <iostream>

using namespace std;

class AudioSource {
    public:
        AudioSource(int buffer_size);

        void *run_read_loop_in_thread() { this->run_read_loop(); return NULL; }

        // run_read_loop continuously reads audio and updates internal state.
        // You should run this in a separate thread, then call copy_audio_data(...) whenever you're ready for audio_data.
        virtual void run_read_loop() = 0;

        // copy_audio_data puts this AudioSource's most recent audio data in dst.
        // dst MUST be at least of size `buffer_size`.
        void copy_audio_data(double *dst);

        int get_buffer_size() const { return buffer_size; }
        virtual int get_audio_sample_rate() = 0;

        void close() { this->atomic_done = true; free(audio_frames_buffer); }

    protected:
        bool done() { return this->atomic_done.load(); }

        int buffer_size;

        pthread_mutex_t audio_frames_buffer_lock;
        double *audio_frames_buffer;

    private:
        atomic<bool> atomic_done;
};

class WAVAudioSource : public AudioSource {
    public:
        WAVAudioSource(int buffer_size, string filename);

        void run_read_loop();

        int get_audio_sample_rate() { return sfinfo.samplerate; }

    private:
        SF_INFO sfinfo;
        SNDFILE *file;
};

/*
class ALSAAudioSource : public AudioSource {
    public:
        ALSAAudioSource(int buffer_size) : AudioSource(buffer_size) {};

        void run_read_loop();

    private:
        char *ra
};
*/
#endif //RASPBERRY_PI_AUDIO_VISUALIZER_AUDIO_SOURCE_HPP
