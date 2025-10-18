#pragma once

#include<xcb/xcb.h>

#include<glad/gl.h>
#include<glad/egl.h>

#include<cglm/cglm.h>

enum SYSTEM_RESULT{
    SYSTEM_RESULT_INVALID=0,

    SYSTEM_RESULT_SUCCESS=1,
};

struct SystemInterface{
    xcb_connection_t*con;
};
enum SYSTEM_RESULT SystemInterface_create(struct SystemInterface*system_interface);
void SystemInterface_destroy(struct SystemInterface*system_interface);

const char*fileContents(const char*filepath);

struct Material{
    uint vertexShader,fragmentShader;
    uint shaderProgram;
};
void Material_create(const char*vertexShaderPath,const char*fragmentShaderPath,struct Material*material);


/* can be drawn */
struct Object{
    uint vao;
    uint vbo;
    int num_vertices;
    int num_faces;

    struct Material*material;
};

struct ObjectVertexAttribute{
    int location;
    int numVertexItems;
    // GL_FLOAT etc.
    int itemType;
    // offset into per-vertex data block
    int64_t itemOffset;
};
struct ObjectVertexInformation{
    int num_vertices;

    // data for all vertices
    void*vertex_data;
    // size of data per vertex
    int stride;

    int numVertexAttributes;
    struct ObjectVertexAttribute*vertexAttributes;
};
/**
 * num_vertices: number of vertices
 * vertices: vertex data (3 floats per vertex!)
 * num_faces: number of faces (triangles)
 * faces: face data (3 uints per face!)
 */
void Object_create(
    int num_faces,
    uint*faces,

    struct ObjectVertexInformation *vertex_info,
    
    struct Material*material,

    struct Object*object
);
void Object_draw(struct Object*object);


struct Window{
    struct SystemInterface*system_interface;

    xcb_screen_t*screen;
    int window_id;

    vec2 size;

    EGLDisplay egl_display;
    EGLConfig egl_config;
    EGLSurface egl_surface;
    EGLContext egl_context;
};

struct WindowOptions{
    int width,height;
    // can be null (for no title)
    const char*title;
    // currently ignored
    bool decorated;
};
void Window_create(struct WindowOptions*options,struct SystemInterface*system_interface,struct Window*window);
// call this before drawing objects
void Window_prepareDrawing(struct Window*window);
void Window_prepareDrawing(struct Window*window);
// call this after drawing object
void Window_finishDrawing(struct Window*window);
void Window_setTitle(struct Window*window,const char*title);
void Window_destroy(struct Window*window);

enum XCB_KEYCODE{
    XCB_KEYCODE_ESCAPE=9,

    XCB_KEYCODE_1=10,
    XCB_KEYCODE_2=11,
    XCB_KEYCODE_3=12,
    XCB_KEYCODE_4=13,
    XCB_KEYCODE_5=14,
    XCB_KEYCODE_6=15,
    XCB_KEYCODE_7=16,
    XCB_KEYCODE_8=17,
    XCB_KEYCODE_9=18,
    XCB_KEYCODE_0=19,

    XCB_KEYCODE_MINUS=20,
    XCB_KEYCODE_EQUAL=21,
    XCB_KEYCODE_BACKSPACE=22,
    XCB_KEYCODE_TAB=23,

    XCB_KEYCODE_Q=24,
    XCB_KEYCODE_W=25,
    XCB_KEYCODE_E=26,
    XCB_KEYCODE_R=27,
    XCB_KEYCODE_T=28,
    XCB_KEYCODE_Y=29,
    XCB_KEYCODE_U=30,
    XCB_KEYCODE_I=31,
    XCB_KEYCODE_O=32,
    XCB_KEYCODE_P=33,
    XCB_KEYCODE_BRACKETLEFT=34,
    XCB_KEYCODE_BRACKETRIGHT=35,
    XCB_KEYCODE_RETURN=36,
    XCB_KEYCODE_CONTROL_L=37,

    XCB_KEYCODE_A=38,
    XCB_KEYCODE_S=39,
    XCB_KEYCODE_D=40,
    XCB_KEYCODE_F=41,
    XCB_KEYCODE_G=42,
    XCB_KEYCODE_H=43,
    XCB_KEYCODE_J=44,
    XCB_KEYCODE_K=45,
    XCB_KEYCODE_L=46,
    XCB_KEYCODE_SEMICOLON=47,
    XCB_KEYCODE_APOSTROPHE=48,
    XCB_KEYCODE_GRAVE=49,
    XCB_KEYCODE_SHIFT_L=50,
    XCB_KEYCODE_BACKSLASH=51,

    XCB_KEYCODE_Z=52,
    XCB_KEYCODE_X=53,
    XCB_KEYCODE_C=54,
    XCB_KEYCODE_V=55,
    XCB_KEYCODE_B=56,
    XCB_KEYCODE_N=57,
    XCB_KEYCODE_M=58,
    XCB_KEYCODE_COMMA=59,
    XCB_KEYCODE_PERIOD=60,
    XCB_KEYCODE_SLASH=61,
    XCB_KEYCODE_SHIFT_R=62,

    XCB_KEYCODE_KP_MULTIPLY=63,
    XCB_KEYCODE_ALT_L=64,
    XCB_KEYCODE_SPACE=65,
    XCB_KEYCODE_CAPS_LOCK=66,

    XCB_KEYCODE_F1=67,
    XCB_KEYCODE_F2=68,
    XCB_KEYCODE_F3=69,
    XCB_KEYCODE_F4=70,
    XCB_KEYCODE_F5=71,
    XCB_KEYCODE_F6=72,
    XCB_KEYCODE_F7=73,
    XCB_KEYCODE_F8=74,
    XCB_KEYCODE_F9=75,
    XCB_KEYCODE_F10=76,
    XCB_KEYCODE_NUM_LOCK=77,
    XCB_KEYCODE_SCROLL_LOCK=78,

    XCB_KEYCODE_KP_HOME=79,
    XCB_KEYCODE_KP_UP=80,
    XCB_KEYCODE_KP_PAGE_UP=81,
    XCB_KEYCODE_KP_SUBTRACT=82,
    XCB_KEYCODE_KP_LEFT=83,
    XCB_KEYCODE_KP_BEGIN=84,
    XCB_KEYCODE_KP_RIGHT=85,
    XCB_KEYCODE_KP_ADD=86,
    XCB_KEYCODE_KP_END=87,
    XCB_KEYCODE_KP_DOWN=88,
    XCB_KEYCODE_KP_PAGE_DOWN=89,
    XCB_KEYCODE_KP_INSERT=90,
    XCB_KEYCODE_KP_DELETE=91,

    XCB_KEYCODE_F11=87,
    XCB_KEYCODE_F12=88,

    XCB_KEYCODE_HOME=110,
    XCB_KEYCODE_UP=111,
    XCB_KEYCODE_PAGE_UP=112,

    XCB_KEYCODE_LEFT=113,
    XCB_KEYCODE_RIGHT=114,

    XCB_KEYCODE_END=115,
    XCB_KEYCODE_DOWN=116,
    XCB_KEYCODE_PAGE_DOWN=117,

    XCB_KEYCODE_INSERT=118,
    XCB_KEYCODE_DELETE=119,

    XCB_KEYCODE_CONTROL_R=105,
    XCB_KEYCODE_ALT_R=108
};
