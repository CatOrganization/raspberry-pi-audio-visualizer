#include <stdio.h>

#include "audio_source.h"

//snd_pcm_format_t audio_format = SND_PCM_FORMAT_S16_LE;

// NOTE: target_sample_rate gets overwritten with the actual sample rate.
snd_pcm_t *init_audio_stream(const char *hw_src, snd_pcm_format_t audio_format, unsigned int *target_sample_rate);
int process_audio_frame(char b1, char b2);

ALSAAudioSource init_alsa_audio_source(ALSAAudioSourceConfig config)
{
    ALSAAudioSource source;

    source.audio_format = config.audio_format;
    source.audio_sample_rate = config.target_sample_rate;
    source.num_frames_to_buffer = config.num_frames_to_buffer;

    source.capture_handle = init_audio_stream(config.hw_src, source.audio_format, &source.audio_sample_rate);

    int audio_format_width = snd_pcm_format_width(config.audio_format);
    fprintf(stdout, "audio format width: %d\n", audio_format_width);

    source.audio_buffer_samples_per_read = source.audio_sample_rate / config.target_reads_per_second;
    source.raw_audio_buffer = malloc(source.audio_buffer_samples_per_read * (audio_format_width / 8));
    source.audio_frames_buffer = malloc(sizeof(double) * source.audio_buffer_samples_per_read * source.num_frames_to_buffer);

    fprintf(stdout, "audio samples per read: %d\n", source.audio_buffer_samples_per_read);
    fprintf(stdout, "audio samples buffered: %d\n", source.audio_buffer_samples_per_read * source.num_frames_to_buffer);

    fprintf(stdout, "CONFIG ----------------------\n");
    fprintf(stdout, "audio_samples_per_read: %d\n", source.audio_buffer_samples_per_read);
    fprintf(stdout, "frames_to_buffer: %d\n", source.num_frames_to_buffer);


    return source;
}

long read_frames(ALSAAudioSource *source)
{
    long err;

    if ((err = snd_pcm_readi(source->capture_handle, source->raw_audio_buffer, source->audio_buffer_samples_per_read)) != source->audio_buffer_samples_per_read) {
        fprintf(stderr, "audio stream read failed: %s", snd_strerror(err));
        return err;
    }

    // Copy the previous sound data over to make room for the new frame of data
    memmove(source->audio_frames_buffer, source->audio_frames_buffer + (source->audio_buffer_samples_per_read), (source->num_frames_to_buffer - 1) * source->audio_buffer_samples_per_read * sizeof(double));

    // Process the new sound data and put it in our buffer
    for (int n = 0; n < source->audio_buffer_samples_per_read * 2; n += 2)
    {
        double processed_frame = process_audio_frame(source->raw_audio_buffer[n], source->raw_audio_buffer[n+1]) / 32767.5;
        int index = (source->audio_buffer_samples_per_read * (source->num_frames_to_buffer - 1)) + n / 2;

        source->audio_frames_buffer[index] = processed_frame;
    }

    return err;
}

snd_pcm_t *init_audio_stream(const char *hw_src, snd_pcm_format_t audio_format, unsigned int *target_sample_rate)
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

int process_audio_frame(char b1, char b2)
{
    const int max = 1 << 16;

    int i = (int) b2;
    i = i << 8;
    i = i | b1;

    if (i > max / 2) {
        return -(max - i);
    }

    return i;
}
