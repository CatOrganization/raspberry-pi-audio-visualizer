#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdlib.h>

#include "visualization.h"
#include "effects.h"
#include "filter.h"
#include "linked_list.h"

#define DVD_LOGO_WIDTH 270
#define DVD_LOGO_HEIGHT 120

static void init();
static void update(double *audio_frames);
static void draw(bool verbose);
static void clean_up();

static const int num_colors = 16;
static const Color colors[] = {
	RAYWHITE, GRAY, YELLOW, GOLD, ORANGE, PINK, RED, MAROON, GREEN, LIME, SKYBLUE, BLUE, PURPLE, VIOLET, BEIGE, MAGENTA
};

static Texture2D logo_texture;
static Vector2 logo_position;
static Vector2 logo_velocity;
static Color logo_color;

static Vector2 logo_max_position;

static Color background_color;

static int frames_till_corner;
static int frame_counter;

static LinkedList firework_list;
static int pending_fireworks;
static int max_new_fireworks_per_frame;

Visualization NewDvdLogoVis()
{
    Visualization vis;
    vis.name = "dvd logo";
    vis.init = init;
    vis.update = update;
    vis.draw = draw;
    vis.clean_up = clean_up;

    return vis;
}

// Calculates how many frames until the dvd logo will hit a corner
static int frames_till_next_corner()
{
    int abs_velocity = absf(logo_velocity.x);
    
    Vector2 vel = logo_velocity;
    Vector2 pos = logo_position;
    Vector2 max_pos = logo_max_position;

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

static void init()
{
    logo_texture = LoadTexture("resources/images/dvd_logo.png");

    logo_position.x = 100;
    logo_position.y = 100;

    logo_velocity.x = -10;
    logo_velocity.y = -10;

    logo_color = get_random_color(80);

    logo_max_position.x = vis_screen_width - DVD_LOGO_WIDTH;
    logo_max_position.y = vis_screen_height - DVD_LOGO_HEIGHT;

    frames_till_corner = frames_till_next_corner();
    frame_counter = 1;

    firework_list.head = NULL;
    firework_list.size = 0;
    pending_fireworks = 0;
}

/**************************************************/
/*               Update Functions                 */
/**************************************************/

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

static void update(double *audio_frames)
{
    // Update the logo position
    logo_position.x += logo_velocity.x;
    logo_position.y += logo_velocity.y;
    
    bool hit_x = false;
    bool hit_y = false;

    if (logo_position.x <= 0)
    {
        logo_position.x = 0;
        logo_velocity.x = -logo_velocity.x;
        logo_color = colors[(int) get_random_number(0, num_colors)];
        hit_x = true;
    }
    else if (logo_position.x >= logo_max_position.x)
    {
        logo_position.x = logo_max_position.x;
        logo_velocity.x = -logo_velocity.x;
        logo_color = colors[(int) get_random_number(0, num_colors)];
        hit_x = true;
    }

    if (logo_position.y <= 0)
    {
        logo_position.y = 0;
        logo_velocity.y = -logo_velocity.y;
        logo_color = colors[(int) get_random_number(0, num_colors)];
        hit_y = true;
    }
    else if (logo_position.y >= logo_max_position.y)
    {
        logo_position.y = logo_max_position.y;
        logo_velocity.y = -logo_velocity.y;
        logo_color = colors[(int) get_random_number(0, num_colors)];
        hit_y = true;
    }

    if (hit_x && hit_y)
    {
        fprintf(stdout, "HIT EM BOTH!!!!!! at %d (expected %d)\n", frame_counter, frames_till_corner);
        frames_till_corner = frames_till_next_corner();
        frame_counter = 0;

        pending_fireworks = 75;
        max_new_fireworks_per_frame = 15;
    }

    // Add fireworks if any are pending
    if (pending_fireworks > 0 && frame_counter % 3 == 0)
    {
        int fireworks_to_add = (int) get_random_number(0, max_new_fireworks_per_frame);
        for (int n = 0; n < fireworks_to_add; n++)
        {
            int x = (int) get_random_number(0, vis_screen_width);
            int y = (int) get_random_number(0, vis_screen_height);
            Color color = get_random_color(1.0f);
            linked_list_add(&firework_list, new_firework(x, y, color, 1.0));
        }

        pending_fireworks -= fireworks_to_add;
        max_new_fireworks_per_frame *= 0.9;
        if (max_new_fireworks_per_frame < 3)
        {
            max_new_fireworks_per_frame = 3;
        }
    }

    linked_list_for_each(&firework_list, &firework_list_update);

    // Find max audio frame value to calculate background intensity
    double max_y = audio_frames[0];
    for (int n = 1; n < vis_audio_buffer_samples; n++)
    {
        if (absf(audio_frames[n]) > max_y)
        {
            max_y = absf(audio_frames[n]);
        }
    }

    background_color = scale_color(logo_color, max_y * 0.9);
    frame_counter++;
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
    DrawTexture(logo_texture, logo_position.x, logo_position.y, logo_color);

    // Draw the border lines that grow as we get closer to the next corner event
    double percent = frame_counter / (double) frames_till_corner;
    Color color = logo_color;
    color.a = 200;

    DrawLineEx((Vector2) { 1, vis_screen_height - 1 }, (Vector2) { percent * vis_screen_width, vis_screen_height - 1 }, 2, color);
    DrawLineEx((Vector2) { vis_screen_width - 1, vis_screen_height - 1 }, (Vector2) { vis_screen_width - 1, (1 - percent) * vis_screen_height }, 2, color);
    DrawLineEx((Vector2) { vis_screen_width - 1, 1 }, (Vector2) { (1 - percent) * vis_screen_width, 1 }, 2, color);
    DrawLineEx((Vector2) { 1, 1 }, (Vector2) { 1, percent * vis_screen_height }, 2, color);

    linked_list_for_each(&firework_list, &firework_list_draw);

    if (verbose)
    {
        char text[128];
        sprintf(text, "%d\n%d", frame_counter, frames_till_corner);
        DrawText(text, 0, 500, 20, RAYWHITE);
    }
}

static void clean_up()
{
    UnloadTexture(logo_texture);
}
