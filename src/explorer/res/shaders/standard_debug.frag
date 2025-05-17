#include "erhe_bxdf.glsl"
#include "erhe_light.glsl"
#include "erhe_srgb.glsl"
#include "erhe_texture.glsl"

layout(location =  0) in vec4       v_position;
layout(location =  1) in vec2       v_texcoord;
layout(location =  2) in vec4       v_color;
layout(location =  3) in vec2       v_aniso_control;
layout(location =  4) in vec3       v_T;
layout(location =  5) in vec3       v_B;
layout(location =  6) in vec3       v_N;
layout(location =  7) in flat uint  v_material_index;
layout(location =  8) in float      v_tangent_scale;
layout(location =  9) in float      v_line_width;
layout(location = 10) in vec4       v_bone_color;
layout(location = 11) in flat uvec2 v_valency_edge_count;

void main() {
    const float bayer_matrix[256] = float[256](
          0.0 / 255.0, 128.0 / 255.0,  32.0 / 255.0, 160.0 / 255.0,   8.0 / 255.0, 136.0 / 255.0,  40.0 / 255.0, 168.0 / 255.0,   2.0 / 255.0, 130.0 / 255.0,  34.0 / 255.0, 162.0 / 255.0,  10.0 / 255.0, 138.0 / 255.0,  42.0 / 255.0, 170.0 / 255.0,
        192.0 / 255.0,  64.0 / 255.0, 224.0 / 255.0,  96.0 / 255.0, 200.0 / 255.0,  72.0 / 255.0, 232.0 / 255.0, 104.0 / 255.0, 194.0 / 255.0,  66.0 / 255.0, 226.0 / 255.0,  98.0 / 255.0, 202.0 / 255.0,  74.0 / 255.0, 234.0 / 255.0, 106.0 / 255.0,
         48.0 / 255.0, 176.0 / 255.0,  16.0 / 255.0, 144.0 / 255.0,  56.0 / 255.0, 184.0 / 255.0,  24.0 / 255.0, 152.0 / 255.0,  50.0 / 255.0, 178.0 / 255.0,  18.0 / 255.0, 146.0 / 255.0,  58.0 / 255.0, 186.0 / 255.0,  26.0 / 255.0, 154.0 / 255.0,
        240.0 / 255.0, 112.0 / 255.0, 208.0 / 255.0,  80.0 / 255.0, 248.0 / 255.0, 120.0 / 255.0, 216.0 / 255.0,  88.0 / 255.0, 242.0 / 255.0, 114.0 / 255.0, 210.0 / 255.0,  82.0 / 255.0, 250.0 / 255.0, 122.0 / 255.0, 218.0 / 255.0,  90.0 / 255.0,
         12.0 / 255.0, 140.0 / 255.0,  44.0 / 255.0, 172.0 / 255.0,   4.0 / 255.0, 132.0 / 255.0,  36.0 / 255.0, 164.0 / 255.0,  14.0 / 255.0, 142.0 / 255.0,  46.0 / 255.0, 174.0 / 255.0,   6.0 / 255.0, 134.0 / 255.0,  38.0 / 255.0, 166.0 / 255.0,
        204.0 / 255.0,  76.0 / 255.0, 236.0 / 255.0, 108.0 / 255.0, 196.0 / 255.0,  68.0 / 255.0, 228.0 / 255.0, 100.0 / 255.0, 206.0 / 255.0,  78.0 / 255.0, 238.0 / 255.0, 110.0 / 255.0, 198.0 / 255.0,  70.0 / 255.0, 230.0 / 255.0, 102.0 / 255.0,
         60.0 / 255.0, 188.0 / 255.0,  28.0 / 255.0, 156.0 / 255.0,  52.0 / 255.0, 180.0 / 255.0,  20.0 / 255.0, 148.0 / 255.0,  62.0 / 255.0, 190.0 / 255.0,  30.0 / 255.0, 158.0 / 255.0,  54.0 / 255.0, 182.0 / 255.0,  22.0 / 255.0, 150.0 / 255.0,
        252.0 / 255.0, 124.0 / 255.0, 220.0 / 255.0,  92.0 / 255.0, 244.0 / 255.0, 116.0 / 255.0, 212.0 / 255.0,  84.0 / 255.0, 254.0 / 255.0, 126.0 / 255.0, 222.0 / 255.0,  94.0 / 255.0, 246.0 / 255.0, 118.0 / 255.0, 214.0 / 255.0,  86.0 / 255.0,
          3.0 / 255.0, 131.0 / 255.0,  35.0 / 255.0, 163.0 / 255.0,  11.0 / 255.0, 139.0 / 255.0,  43.0 / 255.0, 171.0 / 255.0,   1.0 / 255.0, 129.0 / 255.0,  33.0 / 255.0, 161.0 / 255.0,   9.0 / 255.0, 137.0 / 255.0,  41.0 / 255.0, 169.0 / 255.0,
        195.0 / 255.0,  67.0 / 255.0, 227.0 / 255.0,  99.0 / 255.0, 203.0 / 255.0,  75.0 / 255.0, 235.0 / 255.0, 107.0 / 255.0, 193.0 / 255.0,  65.0 / 255.0, 225.0 / 255.0,  97.0 / 255.0, 201.0 / 255.0,  73.0 / 255.0, 233.0 / 255.0, 105.0 / 255.0,
         51.0 / 255.0, 179.0 / 255.0,  19.0 / 255.0, 147.0 / 255.0,  59.0 / 255.0, 187.0 / 255.0,  27.0 / 255.0, 155.0 / 255.0,  49.0 / 255.0, 177.0 / 255.0,  17.0 / 255.0, 145.0 / 255.0,  57.0 / 255.0, 185.0 / 255.0,  25.0 / 255.0, 153.0 / 255.0,
        243.0 / 255.0, 115.0 / 255.0, 211.0 / 255.0,  83.0 / 255.0, 251.0 / 255.0, 123.0 / 255.0, 219.0 / 255.0,  91.0 / 255.0, 241.0 / 255.0, 113.0 / 255.0, 209.0 / 255.0,  81.0 / 255.0, 249.0 / 255.0, 121.0 / 255.0, 217.0 / 255.0,  89.0 / 255.0,
         15.0 / 255.0, 143.0 / 255.0,  47.0 / 255.0, 175.0 / 255.0,   7.0 / 255.0, 135.0 / 255.0,  39.0 / 255.0, 167.0 / 255.0,  13.0 / 255.0, 141.0 / 255.0,  45.0 / 255.0, 173.0 / 255.0,   5.0 / 255.0, 133.0 / 255.0,  37.0 / 255.0, 165.0 / 255.0,
        207.0 / 255.0,  79.0 / 255.0, 239.0 / 255.0, 111.0 / 255.0, 199.0 / 255.0,  71.0 / 255.0, 231.0 / 255.0, 103.0 / 255.0, 205.0 / 255.0,  77.0 / 255.0, 237.0 / 255.0, 109.0 / 255.0, 197.0 / 255.0,  69.0 / 255.0, 229.0 / 255.0, 101.0 / 255.0,
         63.0 / 255.0, 191.0 / 255.0,  31.0 / 255.0, 159.0 / 255.0,  55.0 / 255.0, 183.0 / 255.0,  23.0 / 255.0, 151.0 / 255.0,  61.0 / 255.0, 189.0 / 255.0,  29.0 / 255.0, 157.0 / 255.0,  53.0 / 255.0, 181.0 / 255.0,  21.0 / 255.0, 149.0 / 255.0,
        255.0 / 255.0, 127.0 / 255.0, 223.0 / 255.0,  95.0 / 255.0, 247.0 / 255.0, 119.0 / 255.0, 215.0 / 255.0,  87.0 / 255.0, 253.0 / 255.0, 125.0 / 255.0, 221.0 / 255.0,  93.0 / 255.0, 245.0 / 255.0, 117.0 / 255.0, 213.0 / 255.0,  85.0 / 255.0
    );

    const ivec2 dither_pos   = ivec2(gl_FragCoord.xy) % 16; // Wrap to 16x16 tile
    const int   dither_index = dither_pos.y * 16 + dither_pos.x;
    const float dither_value = bayer_matrix[dither_index];

    Material material = material.materials[v_material_index];
    float opacity = material.opacity;
    if (opacity < dither_value) {
        discard;
    }

    out_color.rgb = srgb_to_linear(material.base_color.rgb);

    const vec3 palette[24] = vec3[24](
        vec3(0.0, 0.0, 0.0), //  0
        vec3(1.0, 0.0, 0.0), //  1
        vec3(0.0, 1.0, 0.0), //  2
        vec3(0.0, 0.0, 1.0), //  3
        vec3(1.0, 1.0, 0.0), //  4
        vec3(0.0, 1.0, 1.0), //  5
        vec3(1.0, 0.0, 1.0), //  6
        vec3(1.0, 1.0, 1.0), //  7
        vec3(0.5, 0.0, 0.0), //  8
        vec3(0.0, 0.5, 0.0), //  9
        vec3(0.0, 0.0, 0.5), // 10
        vec3(0.5, 0.5, 0.0), // 11
        vec3(0.0, 0.5, 0.5), // 12
        vec3(0.5, 0.0, 0.5), // 13
        vec3(0.5, 0.5, 0.5), // 14
        vec3(1.0, 0.5, 0.0), // 15
        vec3(1.0, 0.0, 0.5), // 16
        vec3(0.5, 1.0, 0.0), // 17
        vec3(0.0, 1.0, 0.5), // 18
        vec3(0.5, 0.0, 1.0), // 19
        vec3(0.0, 0.5, 1.0), // 20
        vec3(1.0, 1.0, 0.5), // 21
        vec3(0.5, 1.0, 1.0), // 22
        vec3(1.0, 0.5, 1.0)  // 23
    );

    vec3 view_position_in_world = vec3(
        camera.cameras[0].world_from_node[3][0],
        camera.cameras[0].world_from_node[3][1],
        camera.cameras[0].world_from_node[3][2]
    );

    vec3  V     = normalize(view_position_in_world - v_position.xyz);
    vec3  T0    = normalize(v_T);
    vec3  B0    = normalize(v_B);
    vec3  N     = normalize(v_N);

    Light light          = light_block.lights[0];
    vec3  point_to_light = light.direction_and_outer_spot_cos.xyz;
    vec3  L              = normalize(point_to_light);

    uvec2 base_color_texture         = material.base_color_texture;
    uvec2 metallic_roughness_texture = material.metallic_roughness_texture;

    vec2  T_circular                    = normalize(v_texcoord);
    float circular_anisotropy_magnitude = pow(length(v_texcoord) * 8.0, 0.25);
    // Vertex red channel is used to modulate anisotropy level:
    //   0.0 -- Anisotropic
    //   1.0 -- Isotropic when approaching texcoord (0, 0)
    // Vertex color green channel is used for tangent space selection/control:
    //   0.0 -- Use geometry T and B (from vertex attribute
    //   1.0 -- Use T and B derived from texcoord
    float anisotropy_strength = mix(
        1.0,
        min(1.0, circular_anisotropy_magnitude),
        v_aniso_control.y
    ) * v_aniso_control.x;
    // Mix tangent space geometric .. texcoord generated
    vec3  T                   = circular_anisotropy_magnitude > 0.0 ? mix(T0, T_circular.x * T0 + T_circular.y * B0, v_aniso_control.y) : T0;
    vec3  B                   = circular_anisotropy_magnitude > 0.0 ? mix(B0, T_circular.y * T0 - T_circular.x * B0, v_aniso_control.y) : B0;
    float isotropic_roughness = 0.5 * material.roughness.x + 0.5 * material.roughness.y;
    // Mix roughness based on anisotropy_strength
    float roughness_x         = mix(isotropic_roughness, material.roughness.x, anisotropy_strength);
    float roughness_y         = mix(isotropic_roughness, material.roughness.y, anisotropy_strength);

    mat3 TBN   = mat3(T0, B0, N);
    mat3 TBN_t = transpose(TBN);
    vec3 wo    = normalize(TBN_t * V);
    vec3 wi    = normalize(TBN_t * L);
    vec3 wg    = normalize(TBN_t * N);

#if defined(ERHE_DEBUG_NORMAL)
    out_color.rgb = srgb_to_linear(vec3(0.5) + 0.5 * N);
#endif
#if defined(ERHE_DEBUG_TANGENT)
    out_color.rgb = srgb_to_linear(vec3(0.5) + 0.5 * T0);
#endif
#if defined(ERHE_DEBUG_BITANGENT)
    {
        //vec3 b = normalize(cross(v_N, v_T)) * v_tangent_scale;
        //vec3 b = normalize(cross(v_N, v_T)) * v_tangent_scale;
        //vec3 b = v_TBN[1];
        //float len = length(v_B);
        //if (len < 0.9) {
        //    out_color.rgb = vec3(1.0, 0.0, 0.0);
        //} else { 
        //    out_color.rgb = vec3(0.0, 1.0, 0.0);
        //}
        out_color.rgb = srgb_to_linear(vec3(0.5) + 0.5 * v_B);
        //out_color.rgb = srgb_to_linear(vec3(0.5) + 0.5 * b);
    }
#endif
#if defined(ERHE_DEBUG_TANGENT_W)
    out_color.rgb = srgb_to_linear(vec3(0.5 + 0.5 * v_tangent_scale));
#endif
#if defined(ERHE_DEBUG_VDOTN)
    float V_dot_N = dot(V, N);
    out_color.rgb = srgb_to_linear(vec3(V_dot_N));
#endif
#if defined(ERHE_DEBUG_LDOTN)
    float L_dot_N = dot(L, N);
    out_color.rgb = srgb_to_linear(vec3(L_dot_N));
#endif
#if defined(ERHE_DEBUG_HDOTV)
    vec3  H       = normalize(L + V);
    float H_dot_N = dot(H, N);
    out_color.rgb = srgb_to_linear(vec3(H_dot_N));
#endif
#if defined(ERHE_DEBUG_JOINT_INDICES)
    out_color.rgb = srgb_to_linear(vec3(1.0));
#endif
#if defined(ERHE_DEBUG_JOINT_WEIGHTS)
    out_color.rgb = srgb_to_linear(v_bone_color.rgb);
#endif
#if defined(ERHE_DEBUG_OMEGA_O)
    out_color.rgb = srgb_to_linear(vec3(0.5) + 0.5 * wo);
    out_color.r = 1.0;
#endif
#if defined(ERHE_DEBUG_OMEGA_I)
    out_color.rgb = srgb_to_linear(vec3(0.5) + 0.5 * wi);
    out_color.g = 1.0;
#endif
#if defined(ERHE_DEBUG_OMEGA_G)
    out_color.rgb = srgb_to_linear(vec3(0.5) + 0.5 * wg);
    out_color.b = 1.0;
#endif
#if defined(ERHE_DEBUG_TEXCOORD)
    out_color.rgb = srgb_to_linear(vec3(v_texcoord, 0.0));
#endif
#if defined(ERHE_DEBUG_BASE_COLOR_TEXTURE)
    out_color.rgb = srgb_to_linear(sample_texture(base_color_texture, v_texcoord).rgb);
#endif
#if defined(ERHE_DEBUG_VERTEX_COLOR_RGB)
    out_color.rgb = srgb_to_linear(v_color.rgb);
#endif
#if defined(ERHE_DEBUG_VERTEX_COLOR_ALPHA)
    out_color.rgb = srgb_to_linear(vec3(v_color.a));
#endif
#if defined(ERHE_DEBUG_ANISO_STRENGTH)
    out_color.rgb = srgb_to_linear(vec3(v_aniso_control.x));
#endif
#if defined(ERHE_DEBUG_ANISO_TEXCOORD)
    out_color.rgb = srgb_to_linear(vec3(v_aniso_control.y));
#endif
#if defined(ERHE_DEBUG_VERTEX_VALENCY)
    out_color.rgb = srgb_to_linear(palette[v_valency_edge_count.x % 24]);
#endif
#if defined(ERHE_DEBUG_POLYGON_EDGE_COUNT)
    out_color.rgb = srgb_to_linear(palette[v_valency_edge_count.y % 24]);
#endif
#if defined(ERHE_DEBUG_MISC)
    // Show Draw ID

    // Show Directional light L . N
    out_color.rgb = srgb_to_linear(palette[v_material_index % 24]);

    float N_dot_L = dot(N, L);
    float N_dot_V = dot(N, V);
    float N_dot_T = dot(N, T);
    float N_dot_B = dot(N, B);
    float T_dot_B = dot(T, B);

    out_color.rgb = srgb_to_linear(vec3(N_dot_T, N_dot_B, T_dot_B));

    // Show material
    //Material material = material.materials[v_material_index];
    //out_color.rgb = srgb_to_linear(material.base_color.rgb);

#endif
    out_color.a = 1.0;
}
