#include "audio_source.h"

snd_pcm_t *init_output_stream(SF_INFO sfinfo, unsigned long *frames_per_read)
{
    long err;

    snd_pcm_t *pcm_handle;
    snd_pcm_hw_params_t *params;

    /* Open the PCM device in playback mode */
    if ((err = snd_pcm_open(&pcm_handle, "default", SND_PCM_STREAM_PLAYBACK, 0)) < 0) {
        fprintf(stderr, "cannot open audio device '%s' (%s)\n", "default", snd_strerror(err));
        exit(1);
    }

    /* Allocate parameters object and fill it with default values*/
    snd_pcm_hw_params_alloca(&params);

    if ((err = snd_pcm_hw_params_any(pcm_handle, params)) < 0) {
        fprintf(stderr, "cannot open audio device '%s' (%s)\n", "default", snd_strerror(err));
        exit(1);
    }

    /* Set parameters */
    if ((err = snd_pcm_hw_params_set_access(pcm_handle, params, SND_PCM_ACCESS_RW_INTERLEAVED)) < 0) {
        fprintf(stderr, "cannot open audio device '%s' (%s)\n", "default", snd_strerror(err));
        exit(1);
    }
    if ((err = snd_pcm_hw_params_set_format(pcm_handle, params, SND_PCM_FORMAT_S16_LE)) < 0) {
        fprintf(stderr, "cannot open audio device '%s' (%s)\n", "default", snd_strerror(err));
        exit(1);
    }
    if ((err = snd_pcm_hw_params_set_channels(pcm_handle, params, sfinfo.channels)) < 0) {
        fprintf(stderr, "cannot open audio device '%s' (%s)\n", "default", snd_strerror(err));
        exit(1);
    }
    if ((err = snd_pcm_hw_params_set_rate(pcm_handle, params, sfinfo.samplerate, 0)) < 0) {
        fprintf(stderr, "cannot open audio device '%s' (%s)\n", "default", snd_strerror(err));
        exit(1);
    }

    /* Write parameters */
    snd_pcm_hw_params(pcm_handle, params);

    snd_pcm_hw_params_get_period_size(params, frames_per_read, 0);

    return pcm_handle;
}

WAVFileAudioSource init_wav_audio_source(const char *file)
{
    WAVFileAudioSource source;

    source.file = sf_open(file, SFM_READ, &source.sfinfo);
    source.audio_sample_rate = source.sfinfo.samplerate;
    source.output_handle = init_output_stream(source.sfinfo, &source.audio_buffer_samples_per_read);
    source.audio_frames_buffer = malloc(sizeof(double) * source.sfinfo.channels * source.audio_buffer_samples_per_read);
    source.raw_audio_buffer = malloc(sizeof(short) * source.sfinfo.channels * source.audio_buffer_samples_per_read);

    fprintf(stdout,"Channels: %d\n", source.sfinfo.channels);
    fprintf(stdout,"Sample rate: %d\n", source.sfinfo.samplerate);
    fprintf(stdout,"Sections: %d\n", source.sfinfo.sections);
    fprintf(stdout,"Format: %d\n", source.sfinfo.format);
    fprintf(stdout,"Total samples: %ld\n", source.sfinfo.frames);

    if (source.sfinfo.channels != 1) {
        fprintf(stderr, "WAV file must be mono (single channel)\n");
        exit(1);
    }

    return source;
}

long read_frames_wav(WAVFileAudioSource *source)
{
    int count = sf_readf_double(source->file, source->audio_frames_buffer, source->audio_buffer_samples_per_read);
    if (count > 0) {
//        int pcmrc = snd_pcm_writei(source->output_handle, source->audio_frames_buffer, count);
//        if (pcmrc == -EPIPE) {
//            fprintf(stderr, "Underrun!\n");
//            snd_pcm_prepare(source->output_handle);
//        }
//        else if (pcmrc < 0) {
//            fprintf(stderr, "Error writing to PCM device: %s\n", snd_strerror(pcmrc));
//            exit(1);
//        }
//        else if (pcmrc != count) {
//            fprintf(stderr,"PCM write difffers from PCM read.\n");
//            exit(1);
//        }
//
//        for

        return count;
    }

    // If we're at the end of the file, seek to the beginning and read again.
    sf_seek(source->file, 0, SEEK_SET);
    return sf_readf_double(source->file, source->audio_frames_buffer, source->sfinfo.samplerate / 30);
}
