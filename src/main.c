#include <stdio.h>
#include <alsa/asoundlib.h>

#include "raylib.h"
#include "filter.h"
#include "effects.h"
#include "linked_list.h"


const int screenWidth = 1600;
const int screenHeight = 900;

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

double prev_treble_max = 0;
int firework_cooldown = 0;
void process_treble(LinkedList *firework_list, double treble_max)
{
    firework_cooldown--;

    if (firework_cooldown <= 0 && firework_list->size < 30 && treble_max - prev_treble_max > 0.02)
    {
        int x = (int) ((double) rand() / RAND_MAX * screenWidth);
        int y = (int) ((double) rand() / RAND_MAX * screenHeight);
        linked_list_add(firework_list, new_firework(x, y, get_random_color(1.0f), 1.0));
        firework_cooldown = 5;
    }
}

float wave_y = screenHeight;
float wave_speed = 0;
bool show_wave = false;
void process_bass(WaveLine *wave_line, double bass_max)
{
    if (bass_max > 0.05)
    {
        show_wave = true;
        wave_speed = -100;
    }
    else if (show_wave && wave_speed == 0)
    {
        show_wave = false;
        wave_y = screenHeight;
    }

    if (wave_speed < 0)
    {
        set_wave_line_color(wave_line, (Color) {0, 82, 172, (char) ((wave_speed / -100.0) * 200)});
        wave_y += wave_speed;
        wave_speed += 10; 
        if (wave_y < -100)
        {
            wave_y = screenHeight;
        }
    }
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

int firework_list_update(void *data)
{
    if (!update_firework((Firework *)data))
    {
        // If update returns false, the firework is done. We should free it and delete it from the list
        free(data);
        return false;
    }

    return true;
}

int firework_list_draw(void *data)
{
    draw_firework((Firework *) data);
    return true;
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

    Shader wave_shader = LoadShader(0, TextFormat("resources/shaders/wave.fs", 100));
    int screen_height_loc = GetShaderLocation(wave_shader, "screen_height");
    SetShaderValue(wave_shader, screen_height_loc, &screenHeight, UNIFORM_INT);

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

    LinkedList firework_list;
    firework_list.head = NULL;
    firework_list.size = 0;

    linked_list_add(&firework_list, new_firework(200, 200, GREEN, 1.0));
    
    WaveLine wave_line = init_wave_line(wave_shader, "wave_", screenWidth, screenHeight); 
    set_wave_line_intensity(&wave_line, 100);
    set_wave_line_color(&wave_line, (Color) {0, 82, 172, 200});

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
        
        
        max_y = -1;        
            
        // Process audio values
        for (int i = 0; i < (2 * audio_buffer_frames) - 1; i += 2)
        {
            audio_frames[i/2] = process_audio_frame(raw_audio[i], raw_audio[i+1]) / 32000.0;
            if (absf(audio_frames[i/2]) > max_y)
            {
                max_y = absf(audio_frames[i/2]);
            }
        }
            
        c = interpolate_color(GREEN, BLUE, max_y);
           
        double bass_max = apply_linear_filter(LowPassBassFilter, audio_frames, &bass_filtered_audio_frames, audio_buffer_frames);
        double treble_max = apply_linear_filter(HighPassTrebleFilter, audio_frames, &treble_filtered_audio_frames, audio_buffer_frames);

        process_treble(&firework_list, treble_max);
        process_bass(&wave_line, bass_max);


        if (IsKeyDown(32) && firework_list.size < 50)
        {
            int x = (int) ((double) rand() / RAND_MAX * screenWidth);
            int y = (int) ((double) rand() / RAND_MAX * screenHeight);
            linked_list_add(&firework_list, new_firework(x, y, get_random_color(1.0f), 2.5f));
        }
        
        n++;
        sprintf(str, "fps: %d\nfireworks: %d\nbass_max: %f\ntreble: %f", GetFPS(), firework_list.size, bass_max, treble_max);
        
        linked_list_for_each(&firework_list, &firework_list_update);  
        
        // Draw
        //----------------------------------------------------------------------------------
        BeginDrawing();

            ClearBackground(c);

            if (show_wave) 
            {
                draw_wave_line(&wave_line, wave_y);
            }
            draw_sound_wave(line_points, audio_frames, audio_buffer_frames, 450, screenHeight, inverse_color(c));
            
            linked_list_for_each(&firework_list, &firework_list_draw);

            DrawText(str, 0, 0, 20, RAYWHITE);

        EndDrawing();
        //----------------------------------------------------------------------------------
    }

    // De-Initialization
    //--------------------------------------------------------------------------------------
    UnloadShader(wave_shader);
    CloseWindow();        // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}
