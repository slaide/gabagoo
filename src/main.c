#include<stdlib.h>
#include<stdio.h>
#include<unistd.h>
#include<string.h>

#include<xcb/xcb.h>

#include<glad/gl.h>
#include<glad/egl.h>

#include<cglm/cglm.h>

#define CHECK(COND,ERRMSG){if(!(COND)){fprintf(stderr,"%s\n",ERRMSG);exit(EXIT_FAILURE);}}

static vec2 window_size={1280,720};

enum SYSTEM_RESULT{
    SYSTEM_RESULT_INVALID=0,

    SYSTEM_RESULT_SUCCESS=1,
};

struct SystemInterface{
    xcb_connection_t*con;
};
enum SYSTEM_RESULT SystemInterface_create(struct SystemInterface*system_interface){
    system_interface->con=xcb_connect(nullptr,nullptr);
    CHECK(system_interface->con != nullptr,"xcb connection failed");

    return SYSTEM_RESULT_SUCCESS;
}
void SystemInterface_destroy(struct SystemInterface*system_interface){
    xcb_disconnect(system_interface->con);
}

struct Window{
    struct SystemInterface*system_interface;

    int window_id;
};

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

const char*fileContents(const char*filepath){
    FILE* f=fopen(filepath,"rb");
    fseek(f,0,SEEK_END);
    int filelen=ftell(f);
    fseek(f,0,SEEK_SET);
    char*content=calloc(1,filelen+1);
    fread(content,filelen,1,f);
    fclose(f);
    return content;
}

int main(){
    struct SystemInterface system_interface={};
    SystemInterface_create(&system_interface);

    xcb_screen_t*screen=xcb_setup_roots_iterator(xcb_get_setup(system_interface.con)).data;

    uint32_t mask=XCB_CW_BACK_PIXEL|XCB_CW_EVENT_MASK;
    uint32_t values[2]={
        screen->white_pixel,
        XCB_EVENT_MASK_EXPOSURE |
        XCB_EVENT_MASK_KEY_PRESS |
        XCB_EVENT_MASK_BUTTON_PRESS
    };

    int window_id=xcb_generate_id(system_interface.con);
    xcb_create_window(
        system_interface.con,
        XCB_COPY_FROM_PARENT,
        window_id,
        screen->root,
        0,0,
        window_size[0],window_size[1],
        10,
        XCB_WINDOW_CLASS_INPUT_OUTPUT,
        screen->root_visual,
        mask,
        values
    );

    xcb_map_window(system_interface.con,window_id);

    xcb_flush(system_interface.con);

    CHECK(gladLoaderLoadEGL(EGL_DEFAULT_DISPLAY), "Failed to initialize GLAD EGL\n");

    EGLBoolean egl_res;
    
    EGLDisplay egl_display=eglGetDisplay(EGL_DEFAULT_DISPLAY);
    CHECK(egl_display!=EGL_NO_DISPLAY,"egl_display got no display");

    printf("created window\n");

    EGLint major, minor;
    egl_res=eglInitialize(egl_display,&major,&minor);
    CHECK(egl_res==EGL_TRUE,"eglInitialize failed");

    printf("egl %d.%d\n",major,minor);

    // Reload GLAD with the initialized display to ensure all functions are loaded
    if (!gladLoaderLoadEGL(egl_display)) {
        fprintf(stderr, "Failed to reload GLAD EGL after eglInitialize\n");
        return EXIT_FAILURE;
    }

    // Query supported extensions
    const char *extensions = eglQueryString(egl_display, EGL_EXTENSIONS);
    printf("EGL Extensions:\n");
    const char*extensionsCopy=extensions;
    int extensionsLen=strlen(extensions);
    while(extensions < extensionsCopy+extensionsLen){
        int currentExtensionNameLen=0;
        while(extensions[currentExtensionNameLen]!='\0' && extensions[currentExtensionNameLen]!=' ')currentExtensionNameLen++;
        printf("    %.*s\n",currentExtensionNameLen,extensions);
        extensions+=currentExtensionNameLen+1;
    }
    extensions=extensionsCopy;

    CHECK(strstr(extensions,"EGL_KHR_gl_colorspace"),"opengl colorspace extension unsupported");

    // Query client APIs
    const char *client_apis = eglQueryString(egl_display, EGL_CLIENT_APIS);
    printf("Supported APIs: %s\n", client_apis);
    CHECK(strstr(client_apis,"OpenGL"),"OpenGL api unsupported");

    fflush(stdout);

    // key value pairs
    EGLint config_attribs[]={
        EGL_SURFACE_TYPE, EGL_WINDOW_BIT,

        EGL_RED_SIZE, 8,
        EGL_GREEN_SIZE, 8,
        EGL_BLUE_SIZE, 8,
        EGL_ALPHA_SIZE, 0,
        EGL_DEPTH_SIZE, 16,
        EGL_STENCIL_SIZE, 0,

        EGL_RENDERABLE_TYPE, EGL_OPENGL_BIT,

        // Only hardware-accelerated
        EGL_CONFIG_CAVEAT, EGL_NONE,
        // No multisampling
        EGL_SAMPLES, 0,

        EGL_NONE,
    };

    EGLConfig egl_configs[64];
    EGLint num_configs;
    egl_res=eglChooseConfig(egl_display, config_attribs, egl_configs, 64, &num_configs);
    CHECK(egl_res==EGL_TRUE,"eglChooseConfig failed");

    if(false)
    for (int i = 0; i < num_configs; i++) {
        printf("--- Config %d ---\n", i);

        EGLConfig config=egl_configs[i];

        EGLint buffer_size;
        eglGetConfigAttrib(egl_display, config, EGL_BUFFER_SIZE, &buffer_size);
        printf("  buffer %d\n",buffer_size);

        EGLint red_size,green_size,blue_size,alpha_size,depth_size,stencil_size,color_buffer_type;
        
        eglGetConfigAttrib(egl_display, config, EGL_RED_SIZE, &red_size);
        eglGetConfigAttrib(egl_display, config, EGL_GREEN_SIZE, &green_size);
        eglGetConfigAttrib(egl_display, config, EGL_BLUE_SIZE, &blue_size);
        eglGetConfigAttrib(egl_display, config, EGL_ALPHA_SIZE, &alpha_size);
        eglGetConfigAttrib(egl_display, config, EGL_DEPTH_SIZE, &depth_size);
        eglGetConfigAttrib(egl_display, config, EGL_STENCIL_SIZE, &stencil_size);

        printf("  r%d g%d b%d a%d d%d st%d\n",red_size,green_size,blue_size,alpha_size,depth_size,stencil_size);

        eglGetConfigAttrib(egl_display, config, EGL_COLOR_BUFFER_TYPE, &color_buffer_type);
        printf("  Color buffer type: ");
        switch(color_buffer_type){
            case EGL_RGB_BUFFER: printf("RGB\n"); break;
            case EGL_LUMINANCE_BUFFER: printf("Luminance\n"); break;
            default: printf("unknown\n"); break;
        }
    }
    
    EGLConfig egl_config=egl_configs[0];

    EGLint surface_attribs[3] = {
        EGL_GL_COLORSPACE, EGL_GL_COLORSPACE_SRGB,
        EGL_NONE
    };

    EGLSurface egl_surface=eglCreateWindowSurface(
        egl_display,
        egl_config,
        (EGLNativeWindowType)window_id,
        surface_attribs
    );
    CHECK(egl_surface!=EGL_NO_SURFACE,"eglCreateWindowSurface got no surface");

    EGLint colorspace = 0;
    egl_res=eglQuerySurface(egl_display, egl_surface, EGL_GL_COLORSPACE, &colorspace);
    CHECK(egl_res==EGL_TRUE,"eglQuerySurface gl colorspace failed");
    CHECK(colorspace==EGL_GL_COLORSPACE_SRGB,"colorspace is not srgb");

    egl_res=eglBindAPI(EGL_OPENGL_API);
    CHECK(egl_res==EGL_TRUE,"eglBindAPI failed");

    EGLint context_attribs[] = {
        EGL_CONTEXT_MAJOR_VERSION, 3,
        EGL_CONTEXT_MINOR_VERSION, 3,
        EGL_CONTEXT_OPENGL_PROFILE_MASK, EGL_CONTEXT_OPENGL_CORE_PROFILE_BIT,

        EGL_NONE
    };

    EGLContext egl_context=eglCreateContext(
        egl_display,
        egl_config,
        EGL_NO_CONTEXT,
        context_attribs
    );
    CHECK(egl_context!=EGL_NO_CONTEXT,"eglCreateContext got no context");

    egl_res=eglMakeCurrent(egl_display, egl_surface, egl_surface, egl_context);
    CHECK(egl_res==EGL_TRUE,"eglMakeCurrent failed");

    CHECK(gladLoaderLoadGL(),"failed to glad load opengl\n");

    printf("GL Vendor: %s\n", glGetString(GL_VENDOR));
    printf("GL Renderer: %s\n", glGetString(GL_RENDERER));
    printf("GL Version: %s\n", glGetString(GL_VERSION));

    // Enable sRGB framebuffer
    glEnable(GL_FRAMEBUFFER_SRGB);

    glClearColor(0.2, 0.4, 0.8, 1.0);

    bool window_should_close=false;
    int64_t framenum=0;
    while(true){
        usleep(1000000/30);
        printf("frame %ld\n",framenum++);

        xcb_generic_event_t*raw_event=nullptr;
        while((raw_event = xcb_poll_for_event(system_interface.con))){
            switch(raw_event->response_type & ~0x80){
                case XCB_KEY_PRESS:
                    {
                        xcb_key_press_event_t*event=(xcb_key_press_event_t*)raw_event;
                        if(event->detail==XCB_KEYCODE_ESCAPE){
                            printf("pressed escape. closing.\n");
                            window_should_close=true;
                        }
                    }
                    break;
            }
        }

        if(window_should_close) break;

        glClear(GL_COLOR_BUFFER_BIT);

        float vertices[] = {
            -0.5f, -0.5f, 0.0f,
            0.5f, -0.5f, 0.0f,
            0.0f,  0.5f, 0.0f
        };

        // vertex attribute array (contains pointers to vertex buffers)
        uint vao;
        glGenVertexArrays(1,&vao);
        glBindVertexArray(vao);

        // vertex buffer object
        uint vbo;
        glGenBuffers(1,&vbo);
        // vertex buffer has type array buffer
        glBindBuffer(GL_ARRAY_BUFFER,vbo);
        // upload data
        // STATIC_DRAW: written once, read many times
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

        glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,3*sizeof(float),0);
        glEnableVertexAttribArray(0);

        const char*vertexShaderSource=fileContents("resources/cube.vert.glsl");
        uint vertexShader = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vertexShader, 1, &vertexShaderSource,nullptr);
        glCompileShader(vertexShader);

        const char*fragmentShaderSource=fileContents("resources/cube.frag.glsl");
        uint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fragmentShader, 1, &fragmentShaderSource,nullptr);
        glCompileShader(fragmentShader);

        uint shaderProgram=glCreateProgram();
        glAttachShader(shaderProgram,fragmentShader);
        glAttachShader(shaderProgram,vertexShader);
        glLinkProgram(shaderProgram);

        glUseProgram(shaderProgram);
        glBindVertexArray(vao);
        glDrawArrays(GL_TRIANGLES,0,3);

        egl_res=eglSwapBuffers(egl_display,egl_surface);
        CHECK(egl_res==EGL_TRUE,"eglSwapBuffers failed");
    }

    xcb_destroy_window(system_interface.con,window_id);

    SystemInterface_destroy(&system_interface);

    return EXIT_SUCCESS;
}
