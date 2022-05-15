#include "audio_source.hpp"

#include <iostream>
#include <cstring>
#include <chrono>
#include <thread>

using namespace std;

// Initializes an output handle that writes audio data to be played back by speakers.
snd_pcm_t *init_output_stream(SF_INFO sfinfo)
{
    long err;

    snd_pcm_t *pcm_handle;
    snd_pcm_hw_params_t *params;

    /* Open the PCM device in playback mode */
    if ((err = snd_pcm_open(&pcm_handle, "default", SND_PCM_STREAM_PLAYBACK, 0)) < 0) {
        cout << "cannot open audio device: " << snd_strerror(err) << endl;
        exit(1);
    }

    /* Allocate parameters object and fill it with default values*/
    snd_pcm_hw_params_alloca(&params);

    if ((err = snd_pcm_hw_params_any(pcm_handle, params)) < 0) {
        cout << "cannot open audio device: " << snd_strerror(err) << endl;
        exit(1);
    }

    /* Set parameters */
    if ((err = snd_pcm_hw_params_set_access(pcm_handle, params, SND_PCM_ACCESS_RW_INTERLEAVED)) < 0) {
        cout << "cannot open audio device: " << snd_strerror(err) << endl;
        exit(1);
    }
    if ((err = snd_pcm_hw_params_set_format(pcm_handle, params, SND_PCM_FORMAT_S16_LE)) < 0) {
        cout << "cannot open audio device: " << snd_strerror(err) << endl;
        exit(1);
    }
    if ((err = snd_pcm_hw_params_set_channels(pcm_handle, params, sfinfo.channels)) < 0) {
        cout << "cannot open audio device: " << snd_strerror(err) << endl;
        exit(1);
    }
    if ((err = snd_pcm_hw_params_set_rate(pcm_handle, params, sfinfo.samplerate, 0)) < 0) {
        cout << "cannot open audio device: " << snd_strerror(err) << endl;
        exit(1);
    }

    /* Write parameters */
    snd_pcm_hw_params(pcm_handle, params);
    return pcm_handle;
}

void play_audio(snd_pcm_t *output_handle, short *data, int data_len)
{
    snd_pcm_prepare(output_handle);
    int pcmrc = snd_pcm_writei(output_handle, data, data_len);
    if (pcmrc == -EPIPE) {
        cout << "under-run!" << endl;
    }
    else if (pcmrc < 0) {
        cout << "error writing to PCM device: " << snd_strerror(pcmrc);
        exit(1);
    }
    else if (pcmrc != data_len) {
        cout << "PCM write differs from PCM read; expected " << data_len << " but got " << pcmrc << endl;
        exit(1);
    }
}

WAVAudioSource::WAVAudioSource(int buffer_size, int frame_rate, string filename) : AudioSource(buffer_size, frame_rate)
{
    SF_INFO sfinfo;
    SNDFILE *file;
    
    file = sf_open(filename.c_str(), SFM_READ, &sfinfo);
    this->output_handle = init_output_stream(sfinfo);

    cout << "Channels:      " << sfinfo.channels   << endl;
    cout << "Sample rate:   " << sfinfo.samplerate << endl;
    cout << "Format:        " << sfinfo.format     << endl;
    cout << "Total samples: " << sfinfo.frames     << endl;

    this->samplerate = sfinfo.samplerate;
    this->num_audio_frames = sfinfo.frames;
    this->audio_data = new short[sfinfo.frames];
    
    int frames_read = sf_readf_short(file, this->audio_data, sfinfo.frames);
    if (frames_read != sfinfo.frames)
    {
        cout << "read unexpected amount of frames from file: " << frames_read << "; expected " << sfinfo.frames << endl;
        sf_close(file);
        exit(1);
    }

    sf_close(file);
}

WAVAudioSource::~WAVAudioSource()
{
    delete this->audio_data;
    snd_pcm_close(this->output_handle);
}

// pre-process.
void WAVAudioSource::run_read_loop()
{
    double time_per_frame_ns = (1.0 / this->frame_rate) * 1e9;
    int samples_per_frame = min(this->buffer_size, this->samplerate / this->frame_rate);

    double *processed_frame_buffer = new double[samples_per_frame];

    play_audio(this->output_handle, this->audio_data, this->num_audio_frames);

    int counter = 0;
    while (!this->done())
    {
        long start = chrono::high_resolution_clock::now().time_since_epoch().count();

        // Process the frame of audio data into the format we want.
        for (int i = 0; i < samples_per_frame && (counter * samples_per_frame)+i < this->num_audio_frames; i++)
        {
            processed_frame_buffer[i] = ((double) this->audio_data[(counter * samples_per_frame)+i]) / (1 << 16);
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

        // Copy the audio data where it needs to go
        memcpy(this->audio_frames_buffer + offset, processed_frame_buffer, samples_per_frame * sizeof(double));

        // Done with audio frames buffer
        pthread_mutex_unlock(&this->audio_frames_buffer_lock);

        // Sleep for the remainder of the frame duration
        long frame_time = chrono::high_resolution_clock::now().time_since_epoch().count() - start;
        if (frame_time < time_per_frame_ns)
        {
            this_thread::sleep_for(chrono::duration<double, std::nano>(time_per_frame_ns - frame_time));
        }

        // Increment our counter; if we're done with the audio data, start over at 0
        counter++;
        if (counter * samples_per_frame > this->num_audio_frames)
        {
            cout << "looping audio file" << endl;

            counter = 0;
            play_audio(this->output_handle, this->audio_data, this->num_audio_frames);
        }
    }

    delete processed_frame_buffer;
}
