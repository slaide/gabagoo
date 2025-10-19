#include<stdio.h>
#include<string.h>

#include<util.h>
#include<window.h>

void Window_create(struct WindowOptions*options,struct SystemInterface*system_interface,struct Window*window){
    xcb_screen_t*screen=xcb_setup_roots_iterator(xcb_get_setup(system_interface->con)).data;

    uint32_t mask=XCB_CW_BACK_PIXEL|XCB_CW_EVENT_MASK;
    uint32_t values[2]={
        screen->white_pixel,
        XCB_EVENT_MASK_KEY_PRESS |
        XCB_EVENT_MASK_KEY_RELEASE |
        XCB_EVENT_MASK_BUTTON_PRESS |
        XCB_EVENT_MASK_BUTTON_RELEASE |
        XCB_EVENT_MASK_STRUCTURE_NOTIFY |
        XCB_EVENT_MASK_BUTTON_MOTION |
        XCB_EVENT_MASK_POINTER_MOTION
    };

    int window_id=xcb_generate_id(system_interface->con);
    xcb_create_window(
        system_interface->con,
        XCB_COPY_FROM_PARENT,
        window_id,
        screen->root,
        0,0,
        options->width,options->height,
        10,
        XCB_WINDOW_CLASS_INPUT_OUTPUT,
        screen->root_visual,
        mask,
        values
    );

    xcb_map_window(system_interface->con,window_id);

    xcb_flush(system_interface->con);

    CHECK(gladLoaderLoadEGL(EGL_DEFAULT_DISPLAY), "Failed to initialize GLAD EGL\n");

    EGLBoolean egl_res;

    EGLDisplay egl_display=eglGetDisplay(EGL_DEFAULT_DISPLAY);
    CHECK(egl_display!=EGL_NO_DISPLAY,"egl_display got no display");

    EGLint major, minor;
    egl_res=eglInitialize(egl_display,&major,&minor);
    CHECK(egl_res==EGL_TRUE,"eglInitialize failed");

    printf("egl %d.%d\n",major,minor);

    // Reload GLAD with the initialized display to ensure all functions are loaded
    CHECK(gladLoaderLoadEGL(egl_display), "Failed to reload GLAD EGL after eglInitialize\n");

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
    // enable depth testing
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    // enable depth writing
    glDepthMask(GL_TRUE);

    glClearColor(0.2, 0.4, 0.8, 1.0);

    glViewport(0,0,options->width,options->height);

    *window=(struct Window){
        .system_interface=system_interface,
        .screen=screen,
        .window_id=window_id,
        .size[0]=options->width,
        .size[1]=options->height,
        .egl_config=egl_config,
        .egl_display=egl_display,
        .egl_surface=egl_surface,
        .egl_context=egl_context
    };

    if(options->title)
        Window_setTitle(window,options->title);
}

void Window_prepareDrawing(struct Window*window){
    discard window;

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}
void Window_finishDrawing(struct Window*window){
    auto egl_res=eglSwapBuffers(window->egl_display,window->egl_surface);
    CHECK(egl_res==EGL_TRUE,"eglSwapBuffers failed");
}
void Window_setTitle(struct Window*window,const char*title){
    xcb_change_property(
        window->system_interface->con,
        XCB_PROP_MODE_REPLACE,
        window->window_id,
        XCB_ATOM_WM_NAME,
        XCB_ATOM_STRING,
        8,
        strlen(title),
        title
    );
    xcb_flush(window->system_interface->con);
}

void Window_destroy(struct Window*window){
    xcb_destroy_window(window->system_interface->con,window->window_id);
}
