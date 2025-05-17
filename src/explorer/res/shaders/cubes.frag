in vec3 v_pos_world;

void main() {
    vec4  color = vec4(0.2, 1.0, 0.2, 1.0);
    vec3  L     = normalize(vec3(3.0, 2.0, 1.0));
    vec3  dx    = dFdx(v_pos_world);
    vec3  dy    = dFdy(v_pos_world);
    vec3  N     = normalize(cross(dx, dy));
    float ln    = max(dot(L, N), 0.0);
    out_color.rgb = vec3((0.2 + ln) * color);
    out_color.a = 1;
}
