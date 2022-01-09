#ifndef RASPBERRY_PI_AUDIO_VISUALIZER_AUDIO_SOURCE_H
#define RASPBERRY_PI_AUDIO_VISUALIZER_AUDIO_SOURCE_H

#include <alsa/asoundlib.h>
#include <sndfile.h>

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

typedef struct WAVFileAudioSource {
    SF_INFO sfinfo;
    SNDFILE *file;

    int audio_sample_rate;
    unsigned long audio_buffer_samples_per_read;

    snd_pcm_t *output_handle;

    double *audio_frames_buffer;
    short *raw_audio_buffer;
} WAVFileAudioSource;

WAVFileAudioSource init_wav_audio_source(const char *file);

long read_frames_alsa(ALSAAudioSource *source);
long read_frames_wav(WAVFileAudioSource *source);

int process_audio_frame(char b1, char b2);

#endif //RASPBERRY_PI_AUDIO_VISUALIZER_AUDIO_SOURCE_H
