#include "audio_source.hpp"

#include <iostream>
#include <cstring>
#include <chrono>
#include <thread>

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

WAVAudioSource::WAVAudioSource(int buffer_size, int frame_rate, string filename) : AudioSource(buffer_size, frame_rate)
{
    this->file = sf_open(filename.c_str(), SFM_READ, &this->sfinfo);

    cout << "Channels:      " << this->sfinfo.channels   << endl;
    cout << "Sample rate:   " << this->sfinfo.samplerate << endl;
    cout << "Format:        " << this->sfinfo.format     << endl;
    cout << "Total samples: " << this->sfinfo.frames     << endl;
}

void WAVAudioSource::run_read_loop()
{
    double time_per_frame_ns = (1.0 / this->frame_rate) * 1e9;
    int samples_per_frame = min(this->buffer_size, this->sfinfo.samplerate / this->frame_rate);

    double *single_frame_buffer = new double[samples_per_frame];

    while (!this->done())
    {
        long start = chrono::high_resolution_clock::now().time_since_epoch().count();

        // Read the frames out of the file
        int frames_read = sf_readf_double(this->file, single_frame_buffer, samples_per_frame);
        if (frames_read <= 0)
        {
            cout << "error reading wav file: " << frames_read << endl;
            exit(1);
        }

        // Lock the audio buffer since we're about to mess with it
        pthread_mutex_lock(&this->audio_frames_buffer_lock);

        // This is the offset into the audio_frames_buffer that everything for the current frame will start at.
        int offset = max(0, this->buffer_size - samples_per_frame);

        // If the buffer is bigger than the samples_per_frame, move over what was in there before
        if (offset > 0)
        {
            memmove(this->audio_frames_buffer, this->audio_frames_buffer + samples_per_frame, offset * sizeof(double));
        }

        // Do the real copy
        memcpy(this->audio_frames_buffer + offset, single_frame_buffer, samples_per_frame * sizeof(double));

        // Done with audio frames buffer
        pthread_mutex_unlock(&this->audio_frames_buffer_lock);


        long frame_time = chrono::high_resolution_clock::now().time_since_epoch().count() - start;
        if (frame_time < time_per_frame_ns)
        {
//            cout << "sleeping for " << time_per_frame_ns - frame_time << " / " << time_per_frame_ns <<" ns" << endl;
            this_thread::sleep_for(chrono::duration<double, std::nano>(time_per_frame_ns - frame_time));
        }
    }

    delete single_frame_buffer;
}

