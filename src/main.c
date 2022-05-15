#include <stdio.h>
#include <alsa/asoundlib.h>

#include "raylib.h"
#include "audio_source.hpp"
#include "visualization.h"
#include "visualization.hpp"
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

    int buffer_size = 5880/4;
    AudioSource *source = get_audio_source(buffer_size, target_fps, argv[1]);
    pthread_t audio_thread;

    double *local_buff = (double*) malloc(sizeof(double) * buffer_size);

    vis_screen_width = screenWidth;
    vis_screen_height = screenHeight;
    vis_audio_buffer_samples = buffer_size;
    vis_audio_sample_rate = source->get_audio_sample_rate();

    fprintf(stdout, "initializing visualizations\n");

    int num_visualizations = 7;
    int curr_vis = 0;
    VisualizationClass *visualizations[] = {
        new VisualizationWrapper(NewFireworksAndWavesVis()),
        new VisualizationWrapper(NewSoundWaveVis()),
        new VisualizationWrapper(NewDvdLogoVis()),
        new VisualizationWrapper(NewTimeDomainVis()),
        new VisualizationWrapper(NewFrequencyAndWaveVis()),
        new VisualizationWrapper(NewFrequencyDomainVis()),
        new TripleWaveLineVisualization(screenWidth, screenHeight, buffer_size, source->get_audio_sample_rate())
    };

    // Start audio thread
    pthread_create(&audio_thread, NULL, (void *(*)(void *)) &WAVAudioSource::run_read_loop_in_thread, source);

    // Main game loop
    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        // Get new audio data
        source->copy_audio_data(local_buff);

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

        visualizations[curr_vis]->update(local_buff);

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

            visualizations[curr_vis]->draw(verbose_mode);

            if (verbose_mode)
            {
                DrawText(str, 0, 0, 20, RAYWHITE);

                int vis_name_width = MeasureText(visualizations[curr_vis]->name(), 16);
                DrawText(visualizations[curr_vis]->name(), screenWidth - vis_name_width - 10, screenHeight - 20, 16, RAYWHITE);
            }

        EndDrawing();
        //----------------------------------------------------------------------------------
    }

    // De-Initialization
    //--------------------------------------------------------------------------------------
    for (int n = 0; n < num_visualizations; n++)
    {
        delete visualizations[n];
    }

    CloseWindow();        // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    kiss_fft_cleanup();

    source->close();

    pthread_join(audio_thread, NULL);
    free(source);

    return 0;
}
