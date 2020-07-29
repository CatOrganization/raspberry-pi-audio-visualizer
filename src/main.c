#include <stdio.h>
#include <alsa/asoundlib.h>

#include "raylib.h"
#include "filter.h"

snd_pcm_format_t audio_format = SND_PCM_FORMAT_S16_LE;

snd_pcm_t *init_audio_stream(const char *hw_src) 
{
    int err;
    unsigned int rate = 44100;
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

    if ((err = snd_pcm_hw_params_set_rate_near(capture_handle, hw_params, &rate, 0)) < 0) {
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
    static int max = 1 << 16;
    
    int i = (int) b2;
    i = i << 8;
    i = i | b1;

//return i - (1 << 8);

    if (i > max / 2) {
        return -(max - i);
    }

    return i;
}

Color interpolate_color(Color start, Color end, float percent)
{
    int value = percent * 255;
    if (value < 5) value = 0;
        
    return (Color) { 0, value, 0, 255};
}

void draw_sound_wave(Vector2 *line_point_buffer, double *values, int size, int baseline_y, int scale, Color color)
{
    for (int i = 0; i < size; i += 2)
    {
        int y = baseline_y + (values[i] * scale);
        line_point_buffer[i/2].x = i*2;
        line_point_buffer[i/2].y = y;
    }

    DrawLineStrip(line_point_buffer, size/2, color);
}

Color inverse_color(Color c) 
{
    return (Color) {0, 255 - c.g, 0, c.a};
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
    const int screenWidth = 1600;
    const int screenHeight = 900;

    InitWindow(screenWidth, screenHeight, "audio visualizer");

    SetTargetFPS(30);
    //--------------------------------------------------------------------------------------

    HideCursor();
    DisableCursor();

    int err;
    int n = 0;
    char str[400];

    snd_pcm_t *capture_handle = init_audio_stream(argv[1]);
  
    fprintf(stdout, "audio format width: %d\n", snd_pcm_format_width(audio_format));
   
    int audio_buffer_frames = 800;
    char *raw_audio = malloc(audio_buffer_frames * snd_pcm_format_width(audio_format) / 8);
    double *audio_frames = malloc(sizeof(double) * audio_buffer_frames);
    double *bass_filtered_audio_frames = malloc(sizeof(double) * audio_buffer_frames);
    double *treble_filtered_audio_frames = malloc(sizeof(double) * audio_buffer_frames);

    Vector2 *line_points = malloc(sizeof(Vector2) * audio_buffer_frames);

    Color c = RAYWHITE;
    float max_y = 0;

    // Main game loop
    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        // Update
        //----------------------------------------------------------------------------------

	    long int pending_frames = snd_pcm_avail_update(capture_handle);
    	if ((err = snd_pcm_forward(capture_handle, pending_frames)) < 0) {
	        fprintf(stderr, "error skipping forward in stream (%s)\n", snd_strerror(err));
            break;
        }

        if ((err = snd_pcm_readi(capture_handle, raw_audio, audio_buffer_frames)) != audio_buffer_frames) {
            fprintf(stderr, "audio stream read failed: %s", snd_strerror(err));
            break;
        }

        n++;
        sprintf(str, "fps: %d", GetFPS());

        // Draw
        //----------------------------------------------------------------------------------
        BeginDrawing();

            ClearBackground(BLACK);//(c));

            max_y = 0;        
            
            // Process audio values
            for (int i = 0; i < (2 * audio_buffer_frames) - 1; i += 2)
            {
                audio_frames[i/2] = process_audio_frame(raw_audio[i], raw_audio[i+1]) / 32000.0;
            }

            ApplyLinearFilter(LowPassBassFilter, audio_frames, &bass_filtered_audio_frames, audio_buffer_frames);
            ApplyLinearFilter(HighPassTrebleFilter, audio_frames, &treble_filtered_audio_frames, audio_buffer_frames);
            
            draw_sound_wave(line_points, treble_filtered_audio_frames, audio_buffer_frames, 225, 300, RED);
            draw_sound_wave(line_points, audio_frames, audio_buffer_frames, 450, 300, GREEN);
            draw_sound_wave(line_points, bass_filtered_audio_frames, audio_buffer_frames, 675, 300, BLUE);

            //c = interpolate_color(GREEN, BLUE, max_y / screenHeight); //(n % 120) / 120.0f);

            DrawText(str, 0, 0, 20, RAYWHITE);

        EndDrawing();
        //----------------------------------------------------------------------------------
    }

    // De-Initialization
    //--------------------------------------------------------------------------------------
    CloseWindow();        // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}
