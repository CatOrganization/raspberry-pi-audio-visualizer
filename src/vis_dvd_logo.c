#include <math.h>

#include "visualization.h"
#include "effects.h"
#include "filter.h"

#define VIS_DVD_LOGO_WIDTH 265
#define VIS_DVD_LOGO_HEIGHT 118

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

void vis_dvd_logo_init(int width, int height, int audio_frames)
{
    vis_dvd_logo_metadata.screen_width = width;
    vis_dvd_logo_metadata.screen_height = height;
    vis_dvd_logo_metadata.audio_buffer_frames = audio_frames;

    vis_dvd_logo_metadata.logo_texture = LoadTexture("resources/images/dvd_logo.png");

    vis_dvd_logo_metadata.logo_position.x = (width / 2) + get_random_number(-50, 50);
    vis_dvd_logo_metadata.logo_position.y = (height / 2) + get_random_number(-50, 50);

    vis_dvd_logo_metadata.logo_velocity.x = 10;
    vis_dvd_logo_metadata.logo_velocity.y = 10;

    vis_dvd_logo_metadata.logo_color = get_random_color(80);

    vis_dvd_logo_metadata.logo_max_position.x = width - VIS_DVD_LOGO_WIDTH;
    vis_dvd_logo_metadata.logo_max_position.y = height - VIS_DVD_LOGO_HEIGHT;
}

void vis_dvd_logo_update(double *audio_frames)
{
    // Update the logo position
    vis_dvd_logo_metadata.logo_position.x += vis_dvd_logo_metadata.logo_velocity.x;
    vis_dvd_logo_metadata.logo_position.y += vis_dvd_logo_metadata.logo_velocity.y;
    
    if (vis_dvd_logo_metadata.logo_position.x < 0)
    {
        vis_dvd_logo_metadata.logo_position.x = -vis_dvd_logo_metadata.logo_position.x;
        vis_dvd_logo_metadata.logo_velocity.x = -vis_dvd_logo_metadata.logo_velocity.x;
        vis_dvd_logo_metadata.logo_color = get_random_color(80);
    }
    else if (vis_dvd_logo_metadata.logo_position.x > vis_dvd_logo_metadata.logo_max_position.x)
    {
        vis_dvd_logo_metadata.logo_position.x = vis_dvd_logo_metadata.logo_max_position.x + (vis_dvd_logo_metadata.logo_max_position.x - vis_dvd_logo_metadata.logo_position.x);
        vis_dvd_logo_metadata.logo_velocity.x = -vis_dvd_logo_metadata.logo_velocity.x;
        vis_dvd_logo_metadata.logo_color = get_random_color(80);
    }

    if (vis_dvd_logo_metadata.logo_position.y < 0)
    {
        vis_dvd_logo_metadata.logo_position.y = -vis_dvd_logo_metadata.logo_position.y;
        vis_dvd_logo_metadata.logo_velocity.y = -vis_dvd_logo_metadata.logo_velocity.y;
        vis_dvd_logo_metadata.logo_color = get_random_color(80);
    }
    else if (vis_dvd_logo_metadata.logo_position.y > vis_dvd_logo_metadata.logo_max_position.y)
    {
        vis_dvd_logo_metadata.logo_position.y = vis_dvd_logo_metadata.logo_max_position.y + (vis_dvd_logo_metadata.logo_max_position.y - vis_dvd_logo_metadata.logo_position.y);
        vis_dvd_logo_metadata.logo_velocity.y = -vis_dvd_logo_metadata.logo_velocity.y;
        vis_dvd_logo_metadata.logo_color = get_random_color(80);
    }

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
}

void vis_dvd_logo_draw(bool verbose)
{
    ClearBackground(vis_dvd_logo_metadata.background_color);
    DrawTexture(vis_dvd_logo_metadata.logo_texture, vis_dvd_logo_metadata.logo_position.x, vis_dvd_logo_metadata.logo_position.y, vis_dvd_logo_metadata.logo_color);
}

void vis_dvd_logo_clean_up()
{
    UnloadTexture(vis_dvd_logo_metadata.logo_texture);
}
