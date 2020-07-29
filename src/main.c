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
    char *audio_buffer = malloc(audio_buffer_frames * snd_pcm_format_width(audio_format) / 8);

    Vector2 *line_points = malloc(sizeof(Vector2) * audio_buffer_frames);

    FILE *fptr = fopen("output.txt", "w");

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

        if ((err = snd_pcm_readi(capture_handle, audio_buffer, audio_buffer_frames)) != audio_buffer_frames) {
            fprintf(stderr, "audio stream read failed: %s", snd_strerror(err));
            break;
        }

        n++;
        sprintf(str, "fps: %d", GetFPS());

        // Draw
        //----------------------------------------------------------------------------------
        BeginDrawing();

            ClearBackground((c));

            max_y = 0;        
            
            for (int i = 0; i < (2 * audio_buffer_frames); i += 2)
            {
                int audio_value0 = (process_audio_frame(audio_buffer[i-4], audio_buffer[i-3]));                
                int audio_value1 = (process_audio_frame(audio_buffer[i-2], audio_buffer[i-1]))*1;
                int audio_value2 = (process_audio_frame(audio_buffer[i], audio_buffer[i+1]))*1;
                int audio_value3 = (process_audio_frame(audio_buffer[i+2], audio_buffer[i+3]))*1;                
                int audio_value4 = (process_audio_frame(audio_buffer[i+4], audio_buffer[i+5]));
                int audio_value = (audio_value0 + audio_value1 + audio_value2 + audio_value3 + audio_value4) / 5;

                int y = (audio_value / 32000.0f) * (screenHeight);

		        //DrawRectangle(i/2, (screenHeight/2)-(0), 1, y, RED);
                line_points[i/2].x = i;///2;
                line_points[i/2].y = (screenHeight/2)-(y);

                //if (i > 2 && abs(y) < 5) {
                //    line_points[i/2].y = (line_points[(i/2)-1].y + line_points[i/2].y) / 2;
                //}

                if (abs(y) > max_y)
                {
                    max_y = (float) abs(y);
                }

                //fprintf(fptr, "%d,", audio_value2);
            }

            //fprintf(fptr, "\n");
        
            c = interpolate_color(GREEN, BLUE, max_y / screenHeight); //(n % 120) / 120.0f);

            DrawLineStrip(line_points, audio_buffer_frames, inverse_color(c));
            
            //DrawText(str, 0, 0, 20, RAYWHITE);

        EndDrawing();
        //----------------------------------------------------------------------------------
    }

    // De-Initialization
    //--------------------------------------------------------------------------------------
    CloseWindow();        // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    fclose(fptr);

    return 0;
}
