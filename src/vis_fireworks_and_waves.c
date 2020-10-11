#include <stdlib.h>

#include "visualization.h"
#include "linked_list.h"
#include "effects.h"
#include "filter.h"

void vis_fireworks_and_waves_init(int screen_width, int screen_height, int audio_buffer_frames);
void vis_fireworks_and_waves_update(double *audio_frames);
void vis_fireworks_and_waves_draw(bool verbose);
void vis_fireworks_and_waves_clean_up();

typedef struct VisFireworksAndWavesMetadata {
    int screen_height;
    int screen_width;
    int audio_buffer_frames;

    double *bass_filtered_audio_frames;
    double *treble_filtered_audio_frames;

    Vector2 *sound_wave_line_points;
    Color background_color;

    LinkedList firework_list;

    Shader wave_shader;
    int screen_height_shader_loc;

    WaveLine wave_line;

    double prev_treble_max;
    int firework_cooldown;

    float wave_y;
    float wave_speed;
    bool show_wave;
} VisFireworksAndWavesMetadata;

VisFireworksAndWavesMetadata vis_fireworks_and_waves_metadata;

Visualization NewFireworksAndWavesVis()
{
    Visualization vis;
    vis.name = "fireworks and waves";
    vis.init = vis_fireworks_and_waves_init;
    vis.update = vis_fireworks_and_waves_update;
    vis.draw = vis_fireworks_and_waves_draw;
    vis.clean_up = vis_fireworks_and_waves_clean_up;

    return vis;
}

void vis_fireworks_and_waves_init(int width, int height, int audio_frames) 
{
    vis_fireworks_and_waves_metadata.screen_width = width;
    vis_fireworks_and_waves_metadata.screen_height = height;
    vis_fireworks_and_waves_metadata.audio_buffer_frames = audio_frames;

    vis_fireworks_and_waves_metadata.bass_filtered_audio_frames = malloc(sizeof(double) * vis_fireworks_and_waves_metadata.audio_buffer_frames);
    vis_fireworks_and_waves_metadata.treble_filtered_audio_frames = malloc(sizeof(double) * vis_fireworks_and_waves_metadata.audio_buffer_frames);

    vis_fireworks_and_waves_metadata.sound_wave_line_points = malloc(sizeof(Vector2) * vis_fireworks_and_waves_metadata.audio_buffer_frames);

    vis_fireworks_and_waves_metadata.firework_list.head = NULL;
    vis_fireworks_and_waves_metadata.firework_list.size = 0;

    vis_fireworks_and_waves_metadata.wave_shader = LoadShader(0, TextFormat("resources/shaders/wave.fs", 100));
    vis_fireworks_and_waves_metadata.screen_height_shader_loc = GetShaderLocation(vis_fireworks_and_waves_metadata.wave_shader, "screen_height");
    SetShaderValue(vis_fireworks_and_waves_metadata.wave_shader, vis_fireworks_and_waves_metadata.screen_height_shader_loc, &vis_fireworks_and_waves_metadata.screen_height, UNIFORM_INT);

    vis_fireworks_and_waves_metadata.wave_line = init_wave_line(vis_fireworks_and_waves_metadata.wave_shader, "wave_", vis_fireworks_and_waves_metadata.screen_width, vis_fireworks_and_waves_metadata.screen_height);
    set_wave_line_intensity(&vis_fireworks_and_waves_metadata.wave_line, 100);
    set_wave_line_color(&vis_fireworks_and_waves_metadata.wave_line, (Color) {0, 82, 172, 200});
}

/**************************************************/
/*               Update Functions                 */
/**************************************************/


void vis_fireworks_and_waves_process_treble(LinkedList *firework_list, double treble_max)
{
    vis_fireworks_and_waves_metadata.firework_cooldown--;

    if (vis_fireworks_and_waves_metadata.firework_cooldown <= 0 && vis_fireworks_and_waves_metadata.firework_list.size < 30 && treble_max - vis_fireworks_and_waves_metadata.prev_treble_max > 0.02)
    {
        int x = (int) ((double) rand() / RAND_MAX * vis_fireworks_and_waves_metadata.screen_width);
        int y = (int) ((double) rand() / RAND_MAX * vis_fireworks_and_waves_metadata.screen_height);
        linked_list_add(&vis_fireworks_and_waves_metadata.firework_list, new_firework(x, y, get_random_color(1.0f), 1.0));
        vis_fireworks_and_waves_metadata.firework_cooldown = 5;
    }
}

void vis_fireworks_and_waves_process_bass(WaveLine *wave_line, double bass_max)
{
    if (bass_max > 0.05)
    {
        vis_fireworks_and_waves_metadata.show_wave = true;
        vis_fireworks_and_waves_metadata.wave_speed = -100;
    }
    else if (vis_fireworks_and_waves_metadata.show_wave && vis_fireworks_and_waves_metadata.wave_speed == 0)
    {
        vis_fireworks_and_waves_metadata.show_wave = false;
        vis_fireworks_and_waves_metadata.wave_y = vis_fireworks_and_waves_metadata.screen_height;
    }

    if (vis_fireworks_and_waves_metadata.wave_speed < 0)
    {
        set_wave_line_color(&vis_fireworks_and_waves_metadata.wave_line, (Color) {0, 82, 172, (char) ((vis_fireworks_and_waves_metadata.wave_speed / -100.0) * 200)});
        vis_fireworks_and_waves_metadata.wave_y += vis_fireworks_and_waves_metadata.wave_speed;
        vis_fireworks_and_waves_metadata.wave_speed += 10; 
        if (vis_fireworks_and_waves_metadata.wave_y < -100)
        {
            vis_fireworks_and_waves_metadata.wave_y = vis_fireworks_and_waves_metadata.screen_height;
        }
    }
}

int vis_fireworks_and_waves_firework_list_update(void *data)
{
    if (!update_firework((Firework *)data))
    {
        // If update returns false, the firework is done. We should free it and delete it from the list
        free(data);
        return false;
    }

    return true;
}

void vis_fireworks_and_waves_calculate_sound_wave_line_points(double *audio_frames, int baseline_y, float scale)
{
    float horizontal_scale = vis_fireworks_and_waves_metadata.screen_width / vis_fireworks_and_waves_metadata.audio_buffer_frames;
    for (int n = 0; n < vis_fireworks_and_waves_metadata.audio_buffer_frames; n++)
    {
        vis_fireworks_and_waves_metadata.sound_wave_line_points[n].x = n * horizontal_scale;
        vis_fireworks_and_waves_metadata.sound_wave_line_points[n].y = baseline_y + (audio_frames[n] * scale);
    }
}

void vis_fireworks_and_waves_update(double *audio_frames)
{
    // Find max audio frame value
    double max_y = audio_frames[0];
    for (int n = 1; n < vis_fireworks_and_waves_metadata.audio_buffer_frames; n++)
    {
        if (absf(audio_frames[n]) > max_y)
        {
            max_y = absf(audio_frames[n]);
        }
    }

    // Calculate background color
    vis_fireworks_and_waves_metadata.background_color = scale_color(GREEN, max_y);

    vis_fireworks_and_waves_calculate_sound_wave_line_points(audio_frames, vis_fireworks_and_waves_metadata.screen_height / 2, vis_fireworks_and_waves_metadata.screen_height);

    // Filter and process bass + treble
    double bass_max = apply_linear_filter(LowPassBassFilter, audio_frames, &vis_fireworks_and_waves_metadata.bass_filtered_audio_frames, vis_fireworks_and_waves_metadata.audio_buffer_frames);
    double treble_max = apply_linear_filter(HighPassTrebleFilter, audio_frames, &vis_fireworks_and_waves_metadata.treble_filtered_audio_frames, vis_fireworks_and_waves_metadata.audio_buffer_frames);

    vis_fireworks_and_waves_process_treble(&vis_fireworks_and_waves_metadata.firework_list, treble_max);
    vis_fireworks_and_waves_process_bass(&vis_fireworks_and_waves_metadata.wave_line, bass_max);

    // Update fireworks
    linked_list_for_each(&vis_fireworks_and_waves_metadata.firework_list, &vis_fireworks_and_waves_firework_list_update);

    // Extra fireworks for flair
    if (IsKeyDown(32) && vis_fireworks_and_waves_metadata.firework_list.size < 50)
    {
        int x = (int) ((double) rand() / RAND_MAX * vis_fireworks_and_waves_metadata.screen_width);
        int y = (int) ((double) rand() / RAND_MAX * vis_fireworks_and_waves_metadata.screen_height);
        linked_list_add(&vis_fireworks_and_waves_metadata.firework_list, new_firework(x, y, get_random_color(1.0f), 2.5f));
    }
}


/**************************************************/
/*                 Draw Functions                 */
/**************************************************/

int vis_fireworks_and_waves_firework_list_draw(void *data)
{
    draw_firework((Firework *) data);
    return true;
}

void vis_fireworks_and_waves_draw(bool verbose)
{
    ClearBackground(vis_fireworks_and_waves_metadata.background_color);

    if (vis_fireworks_and_waves_metadata.show_wave) 
    {
        draw_wave_line(&vis_fireworks_and_waves_metadata.wave_line, vis_fireworks_and_waves_metadata.wave_y);
    }

    DrawLineStrip(vis_fireworks_and_waves_metadata.sound_wave_line_points, vis_fireworks_and_waves_metadata.audio_buffer_frames, (Color) {0, 255 - vis_fireworks_and_waves_metadata.background_color.g, 0, 255 });
    
    linked_list_for_each(&vis_fireworks_and_waves_metadata.firework_list, &vis_fireworks_and_waves_firework_list_draw);
}

int vis_fireworks_and_waves_clean_up_linked_list_action_func(void *data)
{
    free(data);
    return false;
}

void vis_fireworks_and_waves_clean_up()
{
    free(vis_fireworks_and_waves_metadata.bass_filtered_audio_frames);
    free(vis_fireworks_and_waves_metadata.treble_filtered_audio_frames);
    free(vis_fireworks_and_waves_metadata.sound_wave_line_points);
    linked_list_for_each(&vis_fireworks_and_waves_metadata.firework_list, vis_fireworks_and_waves_clean_up_linked_list_action_func);
    UnloadShader(vis_fireworks_and_waves_metadata.wave_shader);
}