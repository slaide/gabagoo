#pragma once

#include<cglm/cglm.h>
#include<glad/gl.h>
#include<camera.h>

const char*fileContents(const char*filepath);

struct Material{
    uint
        vertexShader,
        fragmentShader;

    uint shaderProgram;

    GLint
        view_loc,
        proj_loc,
        model_loc;
};
void Material_create(const char*vertexShaderPath,const char*fragmentShaderPath,struct Material*material);

struct VertexAttribute{
    int location;
    int numVertexItems;
    // GL_FLOAT etc.
    int itemType;
    // offset into per-vertex data block
    int64_t itemOffset;
};
struct VertexInformation{
    int num_vertices;

    // data for all vertices
    void*vertex_data;
    // size of data per vertex
    int stride;

    int numVertexAttributes;
    struct VertexAttribute*vertexAttributes;
};

struct Mesh{
    uint vao,vbo,ebo;
    int num_vertices;
    int num_faces;
};
void Mesh_create(
    int num_faces,
    uint*faces,

    struct VertexInformation *vertex_info,

    struct Mesh*mesh
);

struct Transform{
    vec3 pos;
    // quaternion
    vec4 rot;
    vec3 scale;
};
void Transform_getModelMatrix(struct Transform*transform,mat4*objet_matrix);

/* can be drawn */
struct Object{
    struct Transform transform;

    struct Mesh*mesh;
    struct Material*material;

    int num_children;
    struct Object**children;
};

/**
 * num_vertices: number of vertices
 * vertices: vertex data (3 floats per vertex!)
 * num_faces: number of faces (triangles)
 * faces: face data (3 uints per face!)
 */
void Object_create(
    struct Mesh*mesh,
    struct Material*material,

    struct Object*object
);
// after changing the transform, this flushes the update to the gpu
void Object_updateTransformMatrix(struct Object*object);
void Object_draw(struct Object*object);
// Draw with 3D camera (uploads view/proj matrices)
void Object_draw3(struct Object*object, struct Camera3*camera);
// Draw with 2D camera (uploads view/proj matrices)
void Object_draw2(struct Object*object, struct Camera2*camera);
void Object_appendChild(struct Object*child,struct Object*parent);
void Object_destroy(struct Object*object);
void Mesh_parseObj(const char*path,struct Mesh*mesh);
