#include <string.h>

#include "effects.h"

WaveLine init_wave_line(Shader wave_line_shader, const char *uniform_prefix, int screen_width, int screen_height)
{
    WaveLine wave_line;
    wave_line.shader = wave_line_shader;
    char str[30] = "";

    strcpy(str, uniform_prefix);
    strcat(str, "y");
    wave_line.uniform_y = GetShaderLocation(wave_line_shader, str);

    strcpy(str, uniform_prefix);
    strcat(str, "intensity");
    wave_line.uniform_intensity = GetShaderLocation(wave_line_shader, str);

    strcpy(str, uniform_prefix);
    strcat(str, "color");
    wave_line.uniform_color = GetShaderLocation(wave_line_shader, str);

    wave_line.screen_width = screen_width;
    wave_line.screen_height = screen_height;

    return wave_line;
}

void set_wave_line_intensity(WaveLine *wave_line, float intensity)
{
    SetShaderValue(wave_line->shader, wave_line->uniform_intensity, &intensity, UNIFORM_FLOAT);
}

void set_wave_line_color(WaveLine *wave_line, Color color)
{
    Vector4 normalized_color = ColorNormalize(color);
    SetShaderValue(wave_line->shader, wave_line->uniform_color, &normalized_color.x, UNIFORM_VEC4);
}

void draw_wave_line(WaveLine *wave_line, float y)
{
    SetShaderValue(wave_line->shader, wave_line->uniform_y, &y, UNIFORM_FLOAT);

    BeginShaderMode(wave_line->shader);
        DrawRectangle(0, 0, wave_line->screen_width, wave_line->screen_height, RAYWHITE);
    EndShaderMode();
}
