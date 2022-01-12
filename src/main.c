#include <stdio.h>
#include <alsa/asoundlib.h>

#include "raylib.h"
#include "audio_source.hpp"
#include "visualization.h"
#include "kiss_fft.h"
#include "pthread.h"

const int screenWidth = 1600;
const int screenHeight = 900;

const int target_fps = 30;

int vis_screen_width;
int vis_screen_height;
int vis_audio_buffer_samples;
unsigned int vis_audio_sample_rate;

void run_command(const char *command, char *output, int output_len)
{
    FILE *fp = popen(command, "r");
    fgets(output, output_len, fp);
    pclose(fp);
}

int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        fprintf(stderr, "You must provider an audio interface as the first arg.\n");
        exit(1);
    }

    // Initialization
    //--------------------------------------------------------------------------------------
    InitWindow(screenWidth, screenHeight, "audio visualizer");
    SetTargetFPS(target_fps);
    //--------------------------------------------------------------------------------------

    // HideCursor();
    // DisableCursor();

    long err;
    int n = 0;
    char str[400];
    char cmd_output[128];
    bool verbose_mode = false;

//    snd_pcm_t *capture_handle = init_audio_stream(argv[1]);
//
//    fprintf(stdout, "audio format width: %d\n", snd_pcm_format_width(audio_format));
//
//    int audio_buffer_samples = audio_sample_rate / target_fps;
//    int frame_buffer_len = 4;
//    char *raw_audio = malloc(audio_buffer_samples * snd_pcm_format_width(audio_format) / 8);
//    double *audio_frames = malloc(sizeof(double) * audio_buffer_samples * frame_buffer_len);
//
//    fprintf(stdout, "audio buffer frames: %d\n", audio_buffer_samples);

/*
    ALSAAudioSourceConfig config;
    config.hw_src = argv[1];
    config.audio_format = SND_PCM_FORMAT_S16_LE;
    config.target_sample_rate = 44100;
    config.target_reads_per_second = target_fps;
    config.num_frames_to_buffer = 1;

    ALSAAudioSource source = init_alsa_audio_source(config);

    WAVFileAudioSource wav_source = init_wav_audio_source("jazz-guitar-mono-signed-int.wav");
*/

    int buffer_size = 1600;
    AudioSource *source = new WAVAudioSource(buffer_size, "jazz-guitar-mono-signed-int.wav");
    pthread_t audio_thread;

    double *local_buff = (double*) malloc(sizeof(double) * buffer_size);

    int num_visualizations = 6;
    int curr_vis = 0;
    Visualization visualizations[] = {
        NewFireworksAndWavesVis(),
        NewSoundWaveVis(),
        NewDvdLogoVis(),
        NewTimeDomainVis(),
        NewFrequencyAndWaveVis(),
        NewFrequencyDomainVis()
    };

    fprintf(stdout, "initializing visualizations\n");

    vis_screen_width = screenWidth;
    vis_screen_height = screenHeight;
    vis_audio_buffer_samples = buffer_size; // source.audio_buffer_samples_per_read * source.num_frames_to_buffer;
    vis_audio_sample_rate = source->get_audio_sample_rate(); // source.audio_sample_rate;

    for (int n = 0; n < num_visualizations; n++)
    {
        fprintf(stdout, "init '%s'\n", visualizations[n].name);
        visualizations[n].init();
    }

    // Start audio thread
    pthread_create(&audio_thread, NULL, (void *(*)(void *)) &WAVAudioSource::run_read_loop_in_thread, source);

    // Main game loop
    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        // Update
        //----------------------------------------------------------------------------------

//        // Process audio
//        if ((err = snd_pcm_readi(source.capture_handle, raw_audio, audio_buffer_samples)) != audio_buffer_samples) {
//            fprintf(stderr, "audio stream read failed: %s", snd_strerror(err));
//            break;
//        }
//
//        // Copy the previous sound data over to make room for the new frame of data
//        memmove(audio_frames, audio_frames + (audio_buffer_samples), (frame_buffer_len - 1) * audio_buffer_samples * sizeof(double));
//        for (int n = 0; n < audio_buffer_samples * 2; n += 2)
//        {
//            audio_frames[(audio_buffer_samples * (frame_buffer_len - 1)) + n / 2] = process_audio_frame2(raw_audio[n], raw_audio[n+1]) / 32000.0;
//        }

//        if ((err = read_frames_alsa(&source)) != source.audio_buffer_samples_per_read) {
//            fprintf(stderr, "audio stream read failed: %s", snd_strerror(err));
//            break;
//        }

        //fprintf(stdout, "getting audio data\n");
        source->copy_audio_data(local_buff);
        //fprintf(stdout, "GOT audio data: %f\n", local_buff[100]);

/*
        if ((err = read_frames_wav(&wav_source)) <= 0) {
            fprintf(stderr, "audio stream read failed: %ld", err);
            break;
        }
*/
        // Process key presses
        int key_pressed = GetKeyPressed();
        if (key_pressed == KEY_RIGHT || key_pressed == (int) 'n')
        {
            curr_vis++;
            if (curr_vis >= num_visualizations) curr_vis = 0;
        }

        if (key_pressed == KEY_LEFT || key_pressed == (int) 'p')
        {
            curr_vis--;
            if (curr_vis < 0) curr_vis = num_visualizations - 1;
        }

        visualizations[curr_vis].update(local_buff);

        // 'v' or 'd' toggles verbose/debug mode
        if (key_pressed == 118 || key_pressed == 100)
        {
            if (verbose_mode) verbose_mode = 0;
            else verbose_mode = 1;
        }

        // only check temp once every 5 seconds
        if (n % (30 * 5) == 0)
        {
            run_command("vcgencmd measure_temp", cmd_output, 128);
        }

        n++;
        sprintf(str, "fps: %d\nkey: %d\n%s", GetFPS(), key_pressed, cmd_output);

        // Draw
        //----------------------------------------------------------------------------------
        BeginDrawing();

            visualizations[curr_vis].draw(verbose_mode);

            if (verbose_mode)
            {
                DrawText(str, 0, 0, 20, RAYWHITE);

                int vis_name_width = MeasureText(visualizations[curr_vis].name, 16);
                DrawText(visualizations[curr_vis].name, screenWidth - vis_name_width - 10, screenHeight - 20, 16, RAYWHITE);
            }

        EndDrawing();
        //----------------------------------------------------------------------------------
    }

    // De-Initialization
    //--------------------------------------------------------------------------------------
    for (int n = 0; n < num_visualizations; n++)
    {
        visualizations[n].clean_up();
    }

    CloseWindow();        // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    kiss_fft_cleanup();

    source->close();

    pthread_join(audio_thread, NULL);
    free(source);

    return 0;
}
