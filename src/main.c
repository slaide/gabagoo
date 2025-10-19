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

    struct VertexAttribute*triangleVertexAttributes=malloc(sizeof(struct VertexAttribute));
    triangleVertexAttributes[0]=(struct VertexAttribute){
        .itemOffset=0,
        .location=0,
        .itemType=GL_FLOAT,
        .numVertexItems=3
    };

    struct VertexInformation triangle_vertex_info={
        .num_vertices=3,
        .stride=3*sizeof(float),
        .vertex_data=triangle_vertices,

        .numVertexAttributes=1,
        .vertexAttributes=triangleVertexAttributes
    };

    struct Mesh triangle_mesh;
    Mesh_create(
        1,faces,
        &triangle_vertex_info,
        &triangle_mesh
    );

    struct Object object;
    Object_create(
        &triangle_mesh,
        &material,
        &object
    );

    float quad_vertices[]={
         .5,  .5, 0,  1, 1,
         .5, -.5, 0,  1, 0,
        -.5, -.5, 0,  0, 0,
        -.5,  .5, 0,  0, 1,
    };
    uint quad_faces[2*3]={
        0,1,2,
        2,3,0,
    };

    struct Material image_material;
    Material_create(
        "resources/img_quad.vert.glsl",
        "resources/img_quad.frag.glsl",
        &image_material
    );

    struct VertexAttribute*quadVertexAttributes=malloc(sizeof(struct VertexAttribute[2]));
    quadVertexAttributes[0]=(struct VertexAttribute){
        .itemOffset=0,
        .location=0,
        .itemType=GL_FLOAT,
        .numVertexItems=3
    };
    quadVertexAttributes[1]=(struct VertexAttribute){
        .itemOffset=3*sizeof(float),
        .location=1,
        .itemType=GL_FLOAT,
        .numVertexItems=2
    };

    struct VertexInformation quad_vertex_info={
        .num_vertices=4,
        .stride=5*sizeof(float),
        .vertex_data=quad_vertices,

        .numVertexAttributes=2,
        .vertexAttributes=quadVertexAttributes
    };

    struct Mesh quad_mesh;
    Mesh_create(
        2,quad_faces,
        &quad_vertex_info,
        &quad_mesh
    );

    struct Object image_object;
    Object_create(
        &quad_mesh,
        &image_material,
        &image_object
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
        Object_draw(&image_object);

        Window_finishDrawing(&window);
    }

    Window_destroy(&window);

    SystemInterface_destroy(&system_interface);

    return EXIT_SUCCESS;
}
