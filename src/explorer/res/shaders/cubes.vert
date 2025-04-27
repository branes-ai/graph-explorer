void main()
{
    uint vx = gl_VertexID;
    uint instance = vx >> 3;

    vec3 instance_pos = instances[instance].position.xyz;
    vec3 view_position_in_world = vec3(
        camera.cameras[0].world_from_node[3][0],
        camera.cameras[0].world_from_node[3][1],
        camera.cameras[0].world_from_node[3][2]
    ); 
    vec3 local_camera_pos = view_position_in_world - instance_pos;

    uvec3 xyz = uvec3(vx & 0x1, (vx & 0x4) >> 2, (vx & 0x2) >> 1);

    if (local_camera_pos.x > 0) xyz.x = 1 - xyz.x;
    if (local_camera_pos.y > 0) xyz.y = 1 - xyz.y;
    if (local_camera_pos.z > 0) xyz.z = 1 - xyz.z;

    vec3 uvw = vec3(xyz);
    vec3 pos = uvw * 2.0 - 1.0;

    mat4 clip_from_world = camera.cameras[0].clip_from_world;
    vec4 position        = vertex[gl_VertexID];

    gl_Position = clip_from_world * pos;
    v_position = position;
}

