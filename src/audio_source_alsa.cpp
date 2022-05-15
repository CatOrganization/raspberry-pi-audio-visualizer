#include "audio_source.hpp"

#include <iostream>

using namespace std;

const snd_pcm_format_t AUDIO_FORMAT = SND_PCM_FORMAT_S16_LE; // Don't change this unless you also change how the raw audio is processed in the read loop.

snd_pcm_t *init_audio_input(const char *hw_src, snd_pcm_format_t audio_format, unsigned int *target_sample_rate)
{
    int err;
    snd_pcm_t *capture_handle;
    snd_pcm_hw_params_t *hw_params;

    fprintf(stdout, "Initializing sound stuff...\n");

    if ((err = snd_pcm_open(&capture_handle, hw_src, SND_PCM_STREAM_CAPTURE, 0)) < 0) {
        fprintf(stderr, "cannot open audio device '%s' (%s)\n", hw_src, snd_strerror(err));
        exit(1);
    }

    if ((err = snd_pcm_hw_params_malloc(&hw_params)) < 0) {
        fprintf(stderr, "cannot allocate hw params (%s)\n", snd_strerror(err));
        exit(1);
    }

    if ((err = snd_pcm_hw_params_any(capture_handle, hw_params)) < 0) {
        fprintf(stderr, "cannot initialize hw params (%s)\n", snd_strerror(err));
        exit(1);
    }

    if ((err = snd_pcm_hw_params_set_access(capture_handle, hw_params, SND_PCM_ACCESS_RW_INTERLEAVED)) < 0) {
        fprintf(stderr, "cannot set hw_params access type (%s)\n", snd_strerror(err));
        exit(1);
    }

    if ((err = snd_pcm_hw_params_set_format(capture_handle, hw_params, audio_format)) < 0) {
        fprintf(stderr, "cannot set audio format (%s)\n", snd_strerror(err));
        exit(1);
    }

    if ((err = snd_pcm_hw_params_set_rate_near(capture_handle, hw_params, target_sample_rate, 0)) < 0) {
        fprintf(stderr, "cannot set sample rate (%s)\n", snd_strerror(err));
        exit(1);
    }

    if ((err = snd_pcm_hw_params_set_channels(capture_handle, hw_params, 1)) < 0) {
        fprintf(stderr, "cannot set num channels (%s)\n", snd_strerror(err));
        exit(1);
    }

    if ((err = snd_pcm_hw_params(capture_handle, hw_params)) < 0) {
        fprintf(stderr, "cannot set params (%s)\n", snd_strerror(err));
        exit(1);
    }

    snd_pcm_hw_params_free(hw_params);

    if ((err = snd_pcm_prepare(capture_handle)) < 0) {
        fprintf(stderr, "cannot prepare audio interface for use (%s)\n", snd_strerror(err));
        exit(1);
    }

    fprintf(stdout, "Sound stuff initialized!\n");

    return capture_handle;
}

double process_audio_frame(char b1, char b2)
{
    const int max = 1 << 16;

    int i = (int) b2;
    i = i << 8;
    i = i | b1;

    if (i > max / 2) {
        return -(max - i);
    }

    return (double) i / (max / 2);
}


ALSAAudioSource::ALSAAudioSource(int buffer_size, int frame_rate, char *hw_src) : AudioSource(buffer_size, frame_rate)
{
    cout << "opening hardware audio source '" << hw_src << "'" << endl;

    this->samplerate = 44100;
    this->input_handle = init_audio_input(hw_src, AUDIO_FORMAT, &this->samplerate);
    this->samples_per_frame = this->samplerate / frame_rate;
}

void ALSAAudioSource::run_read_loop()
{
    int audio_format_width = snd_pcm_format_width(AUDIO_FORMAT);
    int num_raw_audio_frames = this->samples_per_frame * (audio_format_width / 8);
    char *raw_audio_buffer = new char[num_raw_audio_frames];
    double *processed_frame_buffer = new double[this->samples_per_frame];

    long err;

    while (!this->done())
    {
        // Read data from the source
        int frames_read = snd_pcm_readi(this->input_handle, raw_audio_buffer, this->samples_per_frame);
        if (frames_read != this->samples_per_frame)
        {
            cout << "read from hardware source failed: " << snd_strerror(err);
            exit(1);
        }

        // Process data in tmp buffer
        // Note this is a bit hacky and only works if AUDIO_FORMAT is some kind of SHORT format (2 bytes per frame).
        for (int n = 0; n < num_raw_audio_frames; n += 2)
        {
            processed_frame_buffer[n] = process_audio_frame(raw_audio_buffer[n], raw_audio_buffer[n+1]);
        }

        // This is the offset into the audio_frames_buffer that everything for the current frame will start at.
        int offset = max(0, this->buffer_size - this->samples_per_frame);

        // If the buffer is bigger than the samples_per_frame, move over what was in there before
        if (offset > 0)
        {
            memmove(this->audio_frames_buffer, this->audio_frames_buffer + samples_per_frame, offset * sizeof(double));
        }

        // Copy the audio data where it needs to go
        memcpy(this->audio_frames_buffer + offset, processed_frame_buffer, samples_per_frame * sizeof(double));

        // Done with audio frames buffer
        pthread_mutex_unlock(&this->audio_frames_buffer_lock);
    }

    delete raw_audio_buffer;
    delete processed_frame_buffer;
}
