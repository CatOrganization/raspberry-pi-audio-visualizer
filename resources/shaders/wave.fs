#version 120

uniform int screen_height;

uniform float wave_y;
uniform float wave_intensity;
uniform vec4 wave_color;

void main()
{
    vec4 transparent_color = vec4(wave_color.x, wave_color.y, wave_color.z, 0);    

    vec2 screen_pos = vec2(gl_FragCoord.x, float(screen_height) - gl_FragCoord.y);        
    
    float dist_to_line = screen_pos.y - wave_y;
    float intensity = wave_intensity;
    if (dist_to_line < 0.0) intensity *= 0.5;

    dist_to_line = abs(dist_to_line);

    gl_FragColor = mix(wave_color, transparent_color, dist_to_line / intensity);
}
