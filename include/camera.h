#pragma once

#include<cglm/cglm.h>

struct Camera2{
    vec4 aabb;
    mat4 proj_mat,view_mat;
};

struct Camera3{
    float horz_fov;
    float aspect_ratio;

    vec3 pos;
    /*quat*/vec4 rotation;

    mat4 proj_mat,view_mat;
};
