#include <stdlib.h>

#include <cstdio>

#include "visualization.h"
#include "linked_list.h"
#include "effects.h"
#include "filter.h"

static void init();
static void update(double *audio_frames);
static void draw(bool verbose);
static void clean_up();

static double *bass_filtered_audio_frames;
static double *treble_filtered_audio_frames;

static Vector2 *sound_wave_line_points;
static Color background_color;

static LinkedList firework_list;

static Shader wave_shader;
static int screen_height_shader_loc;

static WaveLine wave_line;

static double prev_treble_max;
static int firework_cooldown;

static float wave_y;
static float wave_speed;
static bool show_wave;

Visualization NewFireworksAndWavesVis()
{
    Visualization vis;
    vis.name = "fireworks and waves";
    vis.init = init;
    vis.update = update;
    vis.draw = draw;
    vis.clean_up = clean_up;

    return vis;
}

static void init()
{
    bass_filtered_audio_frames = (double*) malloc(sizeof(double) * vis_audio_buffer_samples);
    treble_filtered_audio_frames = (double*) malloc(sizeof(double) * vis_audio_buffer_samples);

    sound_wave_line_points = (Vector2*) malloc(sizeof(Vector2) * vis_audio_buffer_samples);

    firework_list.head = NULL;
    firework_list.size = 0;

    wave_shader = LoadShader(0, TextFormat("resources/shaders/wave.fs", 100));
    screen_height_shader_loc = GetShaderLocation(wave_shader, "screen_height");
    SetShaderValue(wave_shader, screen_height_shader_loc, &vis_screen_height, UNIFORM_INT);

    wave_line = init_wave_line(wave_shader, "wave_", vis_screen_width, vis_screen_height);
    set_wave_line_intensity(&wave_line, 100);
    set_wave_line_color(&wave_line, (Color) {0, 82, 172, 200});
}

/**************************************************/
/*               Update Functions                 */
/**************************************************/


static void process_treble(LinkedList *firework_list, double treble_max)
{
    firework_cooldown--;

    if (firework_cooldown <= 0 && firework_list->size < 30 && treble_max - prev_treble_max > 0.02)
    {
        int x = (int) ((double) rand() / RAND_MAX * vis_screen_width);
        int y = (int) ((double) rand() / RAND_MAX * vis_screen_height);
        linked_list_add(firework_list, new_firework(x, y, get_random_color(1.0f), 1.0));
        firework_cooldown = 5;
    }
}

static void process_bass(WaveLine *wave_line, double bass_max)
{
    if (bass_max > 0.05)
    {
        show_wave = true;
        wave_speed = -100;
    }
    else if (show_wave && wave_speed == 0)
    {
        show_wave = false;
        wave_y = vis_screen_height;
    }

    if (wave_speed < 0)
    {
        set_wave_line_color(wave_line, (Color) {0, 82, 172, (char) ((wave_speed / -100.0) * 200)});
        wave_y += wave_speed;
        wave_speed += 10;
        if (wave_y < -100)
        {
            wave_y = vis_screen_height;
        }
    }
}

static int firework_list_update(void *data)
{
    if (!update_firework((Firework *)data))
    {
        // If update returns false, the firework is done. We should free it and delete it from the list
        free(data);
        return false;
    }

    return true;
}

static void calculate_sound_wave_line_points(double *audio_frames, int baseline_y, float scale)
{
    float horizontal_scale = (float) vis_screen_width / vis_audio_buffer_samples;
    for (int n = 0; n < vis_audio_buffer_samples; n++)
    {
        sound_wave_line_points[n].x = n * horizontal_scale;
        sound_wave_line_points[n].y = baseline_y + (audio_frames[n] * scale);
    }
}

static void update(double *audio_frames)
{
    // Find max audio frame value
    double max_y = audio_frames[0];
    for (int n = 1; n < vis_audio_buffer_samples; n++)
    {
        if (absf(audio_frames[n]) > max_y)
        {
            max_y = absf(audio_frames[n]);
        }
    }

    // Calculate background color
    background_color = scale_color(GREEN, max_y);

    calculate_sound_wave_line_points(audio_frames, vis_screen_height / 2, vis_screen_height);

    // Filter and process bass + treble
    double bass_max = apply_linear_filter(LowPassBassFilter, audio_frames, &bass_filtered_audio_frames, vis_audio_buffer_samples);
    double treble_max = apply_linear_filter(HighPassTrebleFilter, audio_frames, &treble_filtered_audio_frames, vis_audio_buffer_samples);

    process_treble(&firework_list, treble_max);
    process_bass(&wave_line, bass_max);

    // Update fireworks
    linked_list_for_each(&firework_list, &firework_list_update);

    // Extra fireworks for flair
    if (IsKeyDown(32) && firework_list.size < 50)
    {
        int x = (int) ((double) rand() / RAND_MAX * vis_screen_width);
        int y = (int) ((double) rand() / RAND_MAX * vis_screen_height);
        linked_list_add(&firework_list, new_firework(x, y, get_random_color(1.0f), 2.5f));
    }
}


/**************************************************/
/*                 Draw Functions                 */
/**************************************************/

static int firework_list_draw(void *data)
{
    draw_firework((Firework *) data);
    return true;
}

static void draw(bool verbose)
{
    ClearBackground(background_color);

    if (show_wave)
    {
        draw_wave_line(&wave_line, wave_y);
    }

    DrawLineStrip(sound_wave_line_points, vis_audio_buffer_samples, (Color) {0, 255 - background_color.g, 0, 255 });

    linked_list_for_each(&firework_list, &firework_list_draw);
}

static int clean_up_linked_list_action_func(void *data)
{
    free(data);
    return false;
}

static void clean_up()
{
    free(bass_filtered_audio_frames);
    free(treble_filtered_audio_frames);
    free(sound_wave_line_points);
    linked_list_for_each(&firework_list, clean_up_linked_list_action_func);
    UnloadShader(wave_shader);
}
