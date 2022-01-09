#ifndef RASPBERRY_PI_AUDIO_VISUALIZER_AUDIO_SOURCE_H
#define RASPBERRY_PI_AUDIO_VISUALIZER_AUDIO_SOURCE_H

#include <alsa/asoundlib.h>

typedef struct ALSAAudioSource {
    unsigned int audio_sample_rate;
    snd_pcm_format_t audio_format;

    snd_pcm_t *capture_handle; // Not safe until initialized

    int audio_buffer_samples_per_read;
    int num_frames_to_buffer;

    char *raw_audio_buffer;
    double *audio_frames_buffer;
} ALSAAudioSource;

typedef struct ALSAAudioSourceConfig {
    const char *hw_src;
    snd_pcm_format_t audio_format;
    unsigned int target_sample_rate;
    unsigned int num_frames_to_buffer;
    unsigned int target_reads_per_second;
} ALSAAudioSourceConfig;

ALSAAudioSource init_alsa_audio_source(ALSAAudioSourceConfig config);

long read_frames(ALSAAudioSource *source);

#endif //RASPBERRY_PI_AUDIO_VISUALIZER_AUDIO_SOURCE_H
