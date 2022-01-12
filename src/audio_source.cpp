#include "audio_source.hpp"

#include <iostream>
#include <cstring>
#include <chrono>
#include <thread>

using namespace std;

AudioSource::AudioSource(int buffer_size) : atomic_done{false}, buffer_size{buffer_size}
{
    cout << "init mutex" << endl;
    int err = pthread_mutex_init(&this->audio_frames_buffer_lock, NULL);
    if (err != 0) {
        cout << "error initializing audio frames buffer lock: " << err << endl;
        exit(1);
    }

    this->audio_frames_buffer = (double*) malloc(sizeof(double) * buffer_size);
}

void AudioSource::copy_audio_data(double *dst)
{
    pthread_mutex_lock(&this->audio_frames_buffer_lock);
    memcpy(dst, this->audio_frames_buffer, sizeof(double) * this->buffer_size);
    pthread_mutex_unlock(&this->audio_frames_buffer_lock);
}

WAVAudioSource::WAVAudioSource(int buffer_size, string filename) : AudioSource(buffer_size)
{
    this->file = sf_open(filename.c_str(), SFM_READ, &this->sfinfo);

    cout << "Channels:      " << this->sfinfo.channels   << endl;
    cout << "Sample rate:   " << this->sfinfo.samplerate << endl;
    cout << "Format:        " << this->sfinfo.format     << endl;
    cout << "Total samples: " << this->sfinfo.frames     << endl;
}

void WAVAudioSource::run_read_loop()
{
    double time_per_frame_ns = (((double) this->buffer_size) / ((double) this->sfinfo.samplerate)) * 1e9;

    while (!this->done())
    {
        long start = chrono::high_resolution_clock::now().time_since_epoch().count();

        pthread_mutex_lock(&this->audio_frames_buffer_lock);
        int frames_read = sf_readf_double(this->file, this->audio_frames_buffer, this->buffer_size);
        pthread_mutex_unlock(&this->audio_frames_buffer_lock);

        if (frames_read <= 0)
        {
            cout << "error reading wav file: " << frames_read << endl;
            exit(1);
        }

        long frame_time = chrono::high_resolution_clock::now().time_since_epoch().count() - start;
        if (frame_time < time_per_frame_ns)
        {
//            cout << "sleeping for " << time_per_frame_ns - frame_time << " / " << time_per_frame_ns <<" ns" << endl;
            this_thread::sleep_for(chrono::duration<double, std::nano>(time_per_frame_ns - frame_time));
        }
    }
}

