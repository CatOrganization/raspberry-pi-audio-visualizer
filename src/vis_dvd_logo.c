#include <math.h>
#include <stdio.h>

#include "visualization.h"
#include "effects.h"
#include "filter.h"
#include "linked_list.h"

#define VIS_DVD_LOGO_WIDTH 270
#define VIS_DVD_LOGO_HEIGHT 120

void vis_dvd_logo_init(int screen_width, int screen_height, int audio_buffer_frames);
void vis_dvd_logo_update(double *audio_frames);
void vis_dvd_logo_draw(bool verbose);
void vis_dvd_logo_clean_up();

typedef struct VisDvdLogoMetadata {
    int screen_height;
    int screen_width;
    int audio_buffer_frames;

    Texture2D logo_texture;
    Vector2 logo_position;
    Vector2 logo_velocity;
    Color logo_color;

    Vector2 logo_max_position;

    Color background_color;

    int frames_till_corner;
    int frame_counter;

    LinkedList firework_list;
    int pending_fireworks;
    int max_new_fireworks_per_frame;
} VisDvdLogoMetadata;

VisDvdLogoMetadata vis_dvd_logo_metadata;

Visualization NewDvdLogoVis()
{
    Visualization vis;
    vis.name = "dvd logo";
    vis.init = vis_dvd_logo_init;
    vis.update = vis_dvd_logo_update;
    vis.draw = vis_dvd_logo_draw;
    vis.clean_up = vis_dvd_logo_clean_up;

    return vis;
}

// Calculates how many frames until the dvd logo will hit a corner
int vis_dvd_logo_frames_till_next_corner()
{
    int abs_velocity = absf(vis_dvd_logo_metadata.logo_velocity.x);
    
    Vector2 vel = vis_dvd_logo_metadata.logo_velocity;
    Vector2 pos = vis_dvd_logo_metadata.logo_position;
    Vector2 max_pos = vis_dvd_logo_metadata.logo_max_position;

    int num_attempts = 500;
    int frames_till_corner = 0;
    bool found = false;

    while (!found && num_attempts > 0)
    {
        int frames_till_x, frames_till_y;
        if (vel.x > 0) frames_till_x = (max_pos.x - pos.x) / abs_velocity;
        else frames_till_x = pos.x / abs_velocity;

        if (vel.y > 0) frames_till_y = (max_pos.y - pos.y) / abs_velocity;
        else frames_till_y = pos.y / abs_velocity;

        if (frames_till_x == frames_till_y)
        {
            frames_till_corner += frames_till_x;
            found = true;
            fprintf(stdout, "Prediction: hitting corner in %d frames; num_wall_hits: %d\n", frames_till_corner, 500 - num_attempts);
            break;
        }
        else if (frames_till_x < frames_till_y)
        {
            frames_till_corner += frames_till_x;
            if (vel.x > 0) pos.x = max_pos.x;
            else pos.x = 0;

            vel.x = -vel.x;
            pos.y += vel.y * frames_till_x;
        }
        else // frames_till_y > frames_till_x
        {
            frames_till_corner += frames_till_y;
            if (vel.y > 0) pos.y = max_pos.y;
            else pos.y = 0;

            vel.y = -vel.y;
            pos.x += vel.x * frames_till_y;
        }

        num_attempts--;
    }

    if (!found)
    {
        return -1;
    }

    return frames_till_corner;
}

void vis_dvd_logo_init(int width, int height, int audio_frames)
{
    vis_dvd_logo_metadata.screen_width = width;
    vis_dvd_logo_metadata.screen_height = height;
    vis_dvd_logo_metadata.audio_buffer_frames = audio_frames;

    vis_dvd_logo_metadata.logo_texture = LoadTexture("resources/images/dvd_logo.png");

    vis_dvd_logo_metadata.logo_position.x = 100;
    vis_dvd_logo_metadata.logo_position.y = 100;

    vis_dvd_logo_metadata.logo_velocity.x = -10;
    vis_dvd_logo_metadata.logo_velocity.y = -10;

    vis_dvd_logo_metadata.logo_color = get_random_color(80);

    vis_dvd_logo_metadata.logo_max_position.x = width - VIS_DVD_LOGO_WIDTH;
    vis_dvd_logo_metadata.logo_max_position.y = height - VIS_DVD_LOGO_HEIGHT;

    vis_dvd_logo_metadata.frames_till_corner = vis_dvd_logo_frames_till_next_corner();
    vis_dvd_logo_metadata.frame_counter = 1;

    vis_dvd_logo_metadata.firework_list.head = NULL;
    vis_dvd_logo_metadata.firework_list.size = 0;
    vis_dvd_logo_metadata.pending_fireworks = 0;
}

/**************************************************/
/*               Update Functions                 */
/**************************************************/

int vis_dvd_logo_firework_list_update(void *data)
{
    if (!update_firework((Firework *)data))
    {
        // If update returns false, the firework is done. We should free it and delete it from the list
        free(data);
        return false;
    }

    return true;
}

void vis_dvd_logo_update(double *audio_frames)
{
    // Update the logo position
    vis_dvd_logo_metadata.logo_position.x += vis_dvd_logo_metadata.logo_velocity.x;
    vis_dvd_logo_metadata.logo_position.y += vis_dvd_logo_metadata.logo_velocity.y;
    
    bool hit_x = false;
    bool hit_y = false;

    if (vis_dvd_logo_metadata.logo_position.x <= 0)
    {
        vis_dvd_logo_metadata.logo_position.x = 0;
        vis_dvd_logo_metadata.logo_velocity.x = -vis_dvd_logo_metadata.logo_velocity.x;
        vis_dvd_logo_metadata.logo_color = get_random_color(80);
        hit_x = true;
    }
    else if (vis_dvd_logo_metadata.logo_position.x >= vis_dvd_logo_metadata.logo_max_position.x)
    {
        vis_dvd_logo_metadata.logo_position.x = vis_dvd_logo_metadata.logo_max_position.x;
        vis_dvd_logo_metadata.logo_velocity.x = -vis_dvd_logo_metadata.logo_velocity.x;
        vis_dvd_logo_metadata.logo_color = get_random_color(80);
        hit_x = true;
    }

    if (vis_dvd_logo_metadata.logo_position.y <= 0)
    {
        vis_dvd_logo_metadata.logo_position.y = 0;
        vis_dvd_logo_metadata.logo_velocity.y = -vis_dvd_logo_metadata.logo_velocity.y;
        vis_dvd_logo_metadata.logo_color = get_random_color(80);
        hit_y = true;
    }
    else if (vis_dvd_logo_metadata.logo_position.y >= vis_dvd_logo_metadata.logo_max_position.y)
    {
        vis_dvd_logo_metadata.logo_position.y = vis_dvd_logo_metadata.logo_max_position.y;
        vis_dvd_logo_metadata.logo_velocity.y = -vis_dvd_logo_metadata.logo_velocity.y;
        vis_dvd_logo_metadata.logo_color = get_random_color(80);
        hit_y = true;
    }

    if (hit_x && hit_y)
    {
        fprintf(stdout, "HIT EM BOTH!!!!!! at %d (expected %d)\n", vis_dvd_logo_metadata.frame_counter, vis_dvd_logo_metadata.frames_till_corner);
        vis_dvd_logo_metadata.frames_till_corner = vis_dvd_logo_frames_till_next_corner();
        vis_dvd_logo_metadata.frame_counter = 0;

        vis_dvd_logo_metadata.pending_fireworks = 75;
        vis_dvd_logo_metadata.max_new_fireworks_per_frame = 15;
    }

    // Add fireworks if any are pending
    if (vis_dvd_logo_metadata.pending_fireworks > 0 && vis_dvd_logo_metadata.frame_counter % 3 == 0)
    {
        int fireworks_to_add = (int) get_random_number(0, vis_dvd_logo_metadata.max_new_fireworks_per_frame);
        for (int n = 0; n < fireworks_to_add; n++)
        {
            int x = (int) get_random_number(0, vis_dvd_logo_metadata.screen_width);
            int y = (int) get_random_number(0, vis_dvd_logo_metadata.screen_height);
            Color color = get_random_color(1.0f);
            linked_list_add(&vis_dvd_logo_metadata.firework_list, new_firework(x, y, color, 1.0));
        }

        vis_dvd_logo_metadata.pending_fireworks -= fireworks_to_add;
        vis_dvd_logo_metadata.max_new_fireworks_per_frame *= 0.9;
        if (vis_dvd_logo_metadata.max_new_fireworks_per_frame < 3)
        {
            vis_dvd_logo_metadata.max_new_fireworks_per_frame = 3;
        }
    }

    linked_list_for_each(&vis_dvd_logo_metadata.firework_list, &vis_dvd_logo_firework_list_update);

    // Find max audio frame value to calculate background intensity
    double max_y = audio_frames[0];
    for (int n = 1; n < vis_dvd_logo_metadata.audio_buffer_frames; n++)
    {
        if (absf(audio_frames[n]) > max_y)
        {
            max_y = absf(audio_frames[n]);
        }
    }

    // This formula increases the affect at lower values and tapers off towards max values.
    max_y = pow(max_y - 1, 3) + 1;
    vis_dvd_logo_metadata.background_color = scale_color(vis_dvd_logo_metadata.logo_color, max_y * 0.9);

    vis_dvd_logo_metadata.frame_counter++;
}

/**************************************************/
/*                 Draw Functions                 */
/**************************************************/

int vis_dvd_logo_firework_list_draw(void *data)
{
    draw_firework((Firework *) data);
    return true;
}

void vis_dvd_logo_draw(bool verbose)
{
    ClearBackground(vis_dvd_logo_metadata.background_color);
    DrawTexture(vis_dvd_logo_metadata.logo_texture, vis_dvd_logo_metadata.logo_position.x, vis_dvd_logo_metadata.logo_position.y, vis_dvd_logo_metadata.logo_color);

    double percent = vis_dvd_logo_metadata.frame_counter / (double) vis_dvd_logo_metadata.frames_till_corner;
    int screen_width = vis_dvd_logo_metadata.screen_width;
    int screen_height = vis_dvd_logo_metadata.screen_height;
    Color color = vis_dvd_logo_metadata.logo_color;
    color.a = 200;

    DrawLineEx((Vector2) { 1, screen_height - 1 }, (Vector2) { percent * screen_width, screen_height - 1 }, 2, color);
    DrawLineEx((Vector2) { screen_width - 1, screen_height - 1 }, (Vector2) { screen_width - 1, (1 - percent) * screen_height }, 2, color);
    DrawLineEx((Vector2) { screen_width - 1, 1 }, (Vector2) { (1 - percent) * screen_width, 1 }, 2, color);
    DrawLineEx((Vector2) { 1, 1 }, (Vector2) { 1, percent * screen_height }, 2, color);

    linked_list_for_each(&vis_dvd_logo_metadata.firework_list, &vis_dvd_logo_firework_list_draw);

    if (verbose)
    {
        char text[128];
        sprintf(text, "%d\n%d", vis_dvd_logo_metadata.frame_counter, vis_dvd_logo_metadata.frames_till_corner);
        DrawText(text, 0, 500, 20, RAYWHITE);
    }
}

void vis_dvd_logo_clean_up()
{
    UnloadTexture(vis_dvd_logo_metadata.logo_texture);
}
