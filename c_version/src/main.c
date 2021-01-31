#include <stdio.h>
#include <alsa/asoundlib.h>

#include "raylib.h"
#include "filter.h"
#include "effects.h"
#include "linked_list.h"
#include "visualization.h"
#include "kiss_fft.h"

const int screenWidth = 1600;
const int screenHeight = 900;

const int target_fps = 30;
unsigned int audio_sample_rate = 44100;
snd_pcm_format_t audio_format = SND_PCM_FORMAT_S16_LE;

snd_pcm_t *init_audio_stream(const char *hw_src) 
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

    if ((err = snd_pcm_hw_params_set_rate_near(capture_handle, hw_params, &audio_sample_rate, 0)) < 0) {
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

void run_command(const char *command, char *output, int output_len)
{
    FILE *fp = popen(command, "r");
    fgets(output, output_len, fp);
    pclose(fp);
}

int process_audio_frame(char b1, char b2)
{
    static int max = 1 << 16;

    int i = (int) b2;
    i = i << 8;
    i = i | b1;

    if (i > max / 2) {
        return -(max - i);
    }

    return i;
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

    int err;
    int n = 0;
    char str[400];
    char cmd_output[128];
    bool verbose_mode = false;

    snd_pcm_t *capture_handle = init_audio_stream(argv[1]);
  
    fprintf(stdout, "audio format width: %d\n", snd_pcm_format_width(audio_format));
   
    int audio_buffer_samples = audio_sample_rate / target_fps;
    int frame_buffer_len = 4;
    char *raw_audio = malloc(audio_buffer_samples * snd_pcm_format_width(audio_format) / 8);
    double *audio_frames = malloc(sizeof(double) * audio_buffer_samples * frame_buffer_len);

    fprintf(stdout, "audio buffer frames: %d\n", audio_buffer_samples);

    int num_visualizations = 5;
    int curr_vis = 0;
    Visualization visualizations[] = {
        NewFireworksAndWavesVis(),
        NewSoundWaveVis(),
        NewDvdLogoVis(),
        NewTimeDomainVis(),
        NewFrequencyAndWaveVis()
    };

    fprintf(stdout, "initializing visualizations\n");

    vis_screen_width = screenWidth;
    vis_screen_height = screenHeight;
    vis_audio_buffer_samples = audio_buffer_samples * frame_buffer_len;
    vis_audio_sample_rate = audio_sample_rate;

    for (int n = 0; n < num_visualizations; n++)
    {
        fprintf(stdout, "init '%s'\n", visualizations[n].name);
        visualizations[n].init(screenWidth, screenHeight, audio_buffer_samples);
    }

    // Main game loop
    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        // Update
        //----------------------------------------------------------------------------------

        // Process audio
        if ((err = snd_pcm_readi(capture_handle, raw_audio, audio_buffer_samples)) != audio_buffer_samples) {
            fprintf(stderr, "audio stream read failed: %s", snd_strerror(err));
            break;
        }

        // Copy the previous sound data over to make room for the new frame of data
        memmove(audio_frames, audio_frames + (audio_buffer_samples), (frame_buffer_len - 1) * audio_buffer_samples * sizeof(double));
        for (int n = 0; n < audio_buffer_samples * 2; n += 2)
        {
            audio_frames[(audio_buffer_samples * (frame_buffer_len - 1)) + n / 2] = process_audio_frame(raw_audio[n], raw_audio[n+1]) / 32000.0;
        }

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

        visualizations[curr_vis].update(audio_frames);    

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

    return 0;
}
