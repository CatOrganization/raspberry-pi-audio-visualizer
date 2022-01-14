#include "audio_source.hpp"

#include <iostream>
#include <cstring>

using namespace std;

AudioSource::AudioSource(int buffer_size, int frame_rate) : atomic_done{false}, buffer_size{buffer_size}, frame_rate{frame_rate}
{
    cout << "init mutex" << endl;
    int err = pthread_mutex_init(&this->audio_frames_buffer_lock, NULL);
    if (err != 0) {
        cout << "error initializing audio frames buffer lock: " << err << endl;
        exit(1);
    }

    this->audio_frames_buffer = new double[buffer_size];
}

AudioSource::~AudioSource()
{
    delete this->audio_frames_buffer;
}


void AudioSource::copy_audio_data(double *dst)
{
    pthread_mutex_lock(&this->audio_frames_buffer_lock);
    memcpy(dst, this->audio_frames_buffer, sizeof(double) * this->buffer_size);
    pthread_mutex_unlock(&this->audio_frames_buffer_lock);
}

