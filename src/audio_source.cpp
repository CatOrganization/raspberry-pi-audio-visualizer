#include "audio_source.hpp"

#include <iostream>
#include <cstring>

using namespace std;

AudioSource *get_audio_source(int buffer_size, int frame_rate, char *arg)
{
    string arg_str = string(arg);
    if (arg_str.length() >= 4)
    {
        string last4 = arg_str.substr(arg_str.length()-4);
        if (last4 == ".wav")
        {
            return new WAVAudioSource(buffer_size, frame_rate, arg_str);
        }
    }

    return new ALSAAudioSource(buffer_size, frame_rate, arg);
}

AudioSource::AudioSource(int buffer_size, int frame_rate) : atomic_done{false}, buffer_size{buffer_size}, frame_rate{frame_rate}
{
    int err = pthread_mutex_init(&this->audio_frames_buffer_lock, NULL);
    if (err != 0) {
        cout << "error initializing audio frames buffer lock: " << err << endl;
        exit(1);
    }

    this->audio_frames_buffer = new double[buffer_size];
}

AudioSource::~AudioSource()
{
    pthread_mutex_destroy(&this->audio_frames_buffer_lock);
    delete this->audio_frames_buffer;
}

void AudioSource::copy_audio_data(double *dst)
{
    pthread_mutex_lock(&this->audio_frames_buffer_lock);
    memcpy(dst, this->audio_frames_buffer, sizeof(double) * this->buffer_size);
    pthread_mutex_unlock(&this->audio_frames_buffer_lock);
}

