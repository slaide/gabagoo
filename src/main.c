#include<stdlib.h>
#include<stdio.h>
#include<unistd.h>

#include<xcb/xcb.h>

#include<cglm/cglm.h>

#include<window.h>
#include<object.h>
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

    struct Material imported_material;
    Material_create(
        "resources/obj.vert.glsl",
        "resources/obj.frag.glsl",
        &imported_material
    );

    struct Mesh imported_mesh;
    Mesh_parseObj("resources/pillow.obj",&imported_mesh);

    struct Object imported_object;
    Object_create(
        &imported_mesh,
        &imported_material,
        &imported_object
    );

    // Initialize camera
    struct{
        vec3 pos;
        /*quat*/vec4 rotation;
        float horz_fov;
    }camera={
        .pos={1,1,1},
        .horz_fov=75
    };
    glm_quat_identity(camera.rotation);

    mat4 cam_view_mat, cam_proj_mat;

    vec3 cam_move_axes={0,0,0};
    vec3 cam_move_speed={0.1,0.1,0.1};

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

                        window.size[0]=event->width;
                        window.size[1]=event->height;
                        glViewport(0,0,window.size[0],window.size[1]);
                    }
                    break;
                case XCB_KEY_PRESS:
                    {
                        xcb_key_press_event_t*event=(xcb_key_press_event_t*)raw_event;
                        switch(event->detail){
                            case XCB_KEYCODE_ESCAPE:
                                {
                                    printf("pressed escape. closing.\n");
                                    window_should_close=true;
                                }
                                break;
                            case XCB_KEYCODE_W:
                                {
                                    cam_move_axes[0]=1;
                                }
                                break;
                            case XCB_KEYCODE_S:
                                {
                                    cam_move_axes[0]=-1;
                                }
                                break;
                            case XCB_KEYCODE_A:
                                {
                                    cam_move_axes[1]=1;
                                }
                                break;
                            case XCB_KEYCODE_D:
                                {
                                    cam_move_axes[1]=-1;
                                }
                                break;
                        }
                    }
                    break;
                case XCB_KEY_RELEASE:
                    {
                        xcb_key_release_event_t*event=(xcb_key_release_event_t*)raw_event;
                        switch(event->detail){
                            case XCB_KEYCODE_W:
                                {
                                    cam_move_axes[0]=0;
                                }
                                break;
                            case XCB_KEYCODE_S:
                                {
                                    cam_move_axes[0]=0;
                                }
                                break;
                            case XCB_KEYCODE_A:
                                {
                                    cam_move_axes[1]=0;
                                }
                                break;
                            case XCB_KEYCODE_D:
                                {
                                    cam_move_axes[1]=0;
                                }
                                break;
                        }
                    }
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

        //Object_draw(&object);

        // move camera
        glm_vec3_muladd(cam_move_axes,cam_move_speed,camera.pos);
        // Update camera matrices
        vec3 up = {0.0f, 0.0f, 1.0f};
        vec3 cam_target={0,0,0};
        glm_lookat(camera.pos, cam_target, up, cam_view_mat);

        // Update perspective projection matrix
        float aspect = (float)window.size[0] / (float)window.size[1];
        glm_perspective(glm_rad(camera.horz_fov), aspect, 0.1f, 100.0f, cam_proj_mat);

        // Setup matrices for image_object
        GLint
            view_loc=glGetUniformLocation(image_object.material->shaderProgram,"view"),
            proj_loc=glGetUniformLocation(image_object.material->shaderProgram,"projection"),
            model_loc=glGetUniformLocation(image_object.material->shaderProgram,"model");

        mat4 obj_mat;
        Transform_getModelMatrix(&image_object.transform,&obj_mat);

        // Upload matrices to GPU
        glUniformMatrix4fv(view_loc,1,GL_FALSE,(float*)cam_view_mat);
        glUniformMatrix4fv(proj_loc,1,GL_FALSE,(float*)cam_proj_mat);
        glUniformMatrix4fv(model_loc,1,GL_FALSE,(float*)obj_mat);

        Object_draw(&image_object);

            view_loc=glGetUniformLocation(imported_object.material->shaderProgram,"view"),
            proj_loc=glGetUniformLocation(imported_object.material->shaderProgram,"projection"),
            model_loc=glGetUniformLocation(imported_object.material->shaderProgram,"model");

        Transform_getModelMatrix(&imported_object.transform,&obj_mat);

        glUniformMatrix4fv(view_loc,1,GL_FALSE,(float*)cam_view_mat);
        glUniformMatrix4fv(proj_loc,1,GL_FALSE,(float*)cam_proj_mat);
        glUniformMatrix4fv(model_loc,1,GL_FALSE,(float*)obj_mat);

        Object_draw(&imported_object);

        Window_finishDrawing(&window);
    }

    Window_destroy(&window);

    SystemInterface_destroy(&system_interface);

    return EXIT_SUCCESS;
}
