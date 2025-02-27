out vec3  vs_position;
out vec4  vs_color;
out float vs_line_width;

void main()
{
    mat4 clip_from_world = view.clip_from_world;
    vec4 position        = vec4(a_position.xyz, 1.0);

    gl_Position   = clip_from_world * position;
    vs_position   = a_position.xyz;
    vs_line_width = a_position.w;
    vs_color      = a_color_0;
}
