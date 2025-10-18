#include<stdlib.h>
#include<stdio.h>
#include<unistd.h>
#include<string.h>

#include<xcb/xcb.h>

#include<cglm/cglm.h>

#include<window.h>
#include<util.h>

static float triangle_vertices[] = {
    -0.5f, -0.5f, 0.0f,
    0.5f, -0.5f, 0.0f,
    0.0f,  0.5f, 0.0f
};
static uint faces[]={
    0,1,2,
};

int main(){
    struct SystemInterface system_interface={};
    SystemInterface_create(&system_interface);

    struct WindowOptions options={
        .width=1280,
        .height=720,
        .title="window title"
    };
    struct Window window;
    Window_create(&options,&system_interface,&window);

    struct Material material;
    Material_create(
        "resources/cube.vert.glsl",
        "resources/cube.frag.glsl",
        &material
    );

    struct Object object;
    Object_create(
        3,triangle_vertices,
        1,faces,
        &material,
        &object
    );

    bool window_should_close=false;
    int64_t framenum=0;
    while(true){
        usleep(1000000/30);
        printf("frame %ld\n",framenum++);

        xcb_generic_event_t*raw_event=nullptr;
        while((raw_event = xcb_poll_for_event(system_interface.con))){
            const int event_type=raw_event->response_type & ~0x80;
            switch(event_type){
                // resize (or move)
                case XCB_CONFIGURE_NOTIFY:
                    {
                        xcb_configure_notify_event_t*event=(xcb_configure_notify_event_t*)raw_event;

                        // this event is also triggered on window move, in which case we dont need to resize
                        if(event->width==window.size[0] && event->height==window.size[1])break;

                        printf("got resized to w%d h%d\n",event->width,event->height);
                        window.size[0]=event->width;
                        window.size[1]=event->height;
                        glViewport(0,0,window.size[0],window.size[1]);
                    }
                    break;
                case XCB_KEY_PRESS:
                    {
                        xcb_key_press_event_t*event=(xcb_key_press_event_t*)raw_event;
                        if(event->detail==XCB_KEYCODE_ESCAPE){
                            printf("pressed escape. closing.\n");
                            window_should_close=true;
                        }
                    }
                    break;
                case XCB_KEY_RELEASE:
                    {}
                    break;
                case XCB_BUTTON_PRESS:
                    {}
                    break;
                case XCB_BUTTON_RELEASE:
                    {}
                    break;
                default:
                    printf("unhandled event %d\n",event_type);
            }
        }

        if(window_should_close) break;

        Window_prepareDrawing(&window);

        Object_draw(&object);

        Window_finishDrawing(&window);
    }

    Window_destroy(&window);

    SystemInterface_destroy(&system_interface);

    return EXIT_SUCCESS;
}
