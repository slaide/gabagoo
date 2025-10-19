#include<stdlib.h>
#include<stdio.h>
#include<unistd.h>

#include<cglm/cglm.h>

#include<window.h>
#include<object.h>
#include<camera.h>
#include<util.h>

static float triangle_vertices[] = {
    -0.5f, -0.5f, 0.0f,
    0.5f, -0.5f, 0.0f,
    0.0f,  0.5f, 0.0f
};
static uint faces[]={
    0,1,2,
};

static float quad_vertices[]={
    .5,  .5, 0,  1, 1,
    .5, -.5, 0,  1, 0,
    -.5, -.5, 0,  0, 0,
    -.5,  .5, 0,  0, 1,
};
static uint quad_faces[2*3]={
    0,1,2,
    2,3,0,
};

static float ui_quad_vertices[]={
    50, 80, 0,
    50, 0,  0,
    0,  0,  0,
    0,  80, 0,
};
static uint ui_quad_faces[2*3]={
    0,1,2,
    2,3,0,
};

/**
 * aabb is x0y0x1y1
 */
void Camera2_create(vec4 aabb,struct Camera2*camera){
    glm_vec4_copy(aabb, camera->aabb);

    // Set view matrix to identity (no camera transformations for basic 2D rendering)
    glm_mat4_identity(camera->view_mat);

    // Set orthographic projection for the AABB bounds
    // aabb format: [x0, y0, x1, y1]
    glm_ortho(aabb[0], aabb[2], aabb[1], aabb[3], -1.0f, 1.0f, camera->proj_mat);
}

void Camera2_updateMatrices(struct Camera2*camera, vec4 aabb){
    glm_vec4_copy(aabb, camera->aabb);

    // Set view matrix to identity (no camera transformations for basic 2D rendering)
    glm_mat4_identity(camera->view_mat);

    // Update orthographic projection for the new AABB bounds
    // aabb format: [x0, y0, x1, y1]
    glm_ortho(aabb[0], aabb[2], aabb[1], aabb[3], -1.0f, 1.0f, camera->proj_mat);
}

void Camera3_create(
    float horz_fov,
    float aspect_ratio,

    struct Camera3*camera
){
    *camera=(struct Camera3){
        .horz_fov=horz_fov,
        .aspect_ratio=aspect_ratio
    };

    glm_quat_identity(camera->rotation);
}

static void quat_right(vec4 quat, vec3 out){
    // Extract right vector (+X axis) by rotating (1, 0, 0)
    vec3 right = {1.0f, 0.0f, 0.0f};
    glm_quat_rotatev(quat, right, out);
}

static void quat_forward(vec4 quat, vec3 out){
    // Extract forward vector (+Y axis) by rotating (0, 1, 0)
    vec3 forward = {0.0f, 1.0f, 0.0f};
    glm_quat_rotatev(quat, forward, out);
}

static void quat_up(vec4 quat, vec3 out){
    // Extract up vector (+Z axis) by rotating (0, 0, 1)
    vec3 up = {0.0f, 0.0f, 1.0f};
    glm_quat_rotatev(quat, up, out);
}

void Camera3_lookAt(struct Camera3*camera, vec3 target){
    // Calculate forward direction (from camera to target)
    vec3 forward;
    glm_vec3_sub(target, camera->pos, forward);
    glm_vec3_normalize(forward);

    // World up vector (Z-up)
    vec3 world_up = {0.0f, 0.0f, 1.0f};

    // Calculate right vector as cross product of forward × world_up
    vec3 right;
    glm_vec3_cross(forward, world_up, right);
    float right_len = glm_vec3_norm(right);

    // Handle edge case: forward is parallel to world_up
    if (right_len < 0.0001f) {
        // Forward is pointing straight up or down
        // Use a different reference axis
        vec3 forward_alt = {0.0f, 1.0f, 0.0f};
        glm_vec3_cross(forward, forward_alt, right);
    }
    glm_vec3_normalize(right);

    // Recalculate up as right × forward to ensure orthonormality
    vec3 up;
    glm_vec3_cross(right, forward, up);
    glm_vec3_normalize(up);

    // Build rotation matrix from basis vectors
    // Camera's local frame: right = +X, up = +Z (world), forward = +Y
    mat3 basis = {
        {right[0], right[1], right[2]},
        {forward[0], forward[1], forward[2]},
        {up[0], up[1], up[2]}
    };

    // Convert rotation matrix to quaternion
    glm_mat3_quat(basis, camera->rotation);
}

void Camera3_updateMatrices(struct Camera3*camera){
    // Calculate forward vector from camera rotation
    vec3 forward;
    quat_forward(camera->rotation, forward);

    // Calculate camera target point
    vec3 cam_target;
    glm_vec3_add(camera->pos, forward, cam_target);

    // Calculate up vector by rotating (0, 0, 1) by the camera's rotation
    vec3 up = {0.0f, 0.0f, 1.0f};
    glm_quat_rotatev(camera->rotation, up, up);

    // Build view matrix from position, target, and up
    glm_lookat(camera->pos, cam_target, up, camera->view_mat);

    // Update perspective projection matrix
    glm_perspective(glm_rad(camera->horz_fov), camera->aspect_ratio, 0.1f, 100.0f, camera->proj_mat);
}

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

    // Create UI material
    struct Material ui_material;
    Material_create(
        "resources/ui_quad.vert.glsl",
        "resources/ui_quad.frag.glsl",
        &ui_material
    );

    // Create UI quad mesh
    struct VertexAttribute*ui_vertex_attributes=malloc(sizeof(struct VertexAttribute));
    ui_vertex_attributes[0]=(struct VertexAttribute){
        .itemOffset=0,
        .location=0,
        .itemType=GL_FLOAT,
        .numVertexItems=3
    };

    struct VertexInformation ui_vertex_info={
        .num_vertices=4,
        .stride=3*sizeof(float),
        .vertex_data=ui_quad_vertices,

        .numVertexAttributes=1,
        .vertexAttributes=ui_vertex_attributes
    };

    struct Mesh ui_mesh;
    Mesh_create(
        2,ui_quad_faces,
        &ui_vertex_info,
        &ui_mesh
    );

    struct Object ui_object;
    Object_create(
        &ui_mesh,
        &ui_material,
        &ui_object
    );

    // Initialize camera
    struct Camera3 camera={};
    Camera3_create(
        75,
        (float)window.size[0] / (float)window.size[1],
        &camera
    );

    vec3 cam_position = {0.0f, 1.0f, 1.0f};
    glm_vec3_copy(cam_position, camera.pos);
    vec3 cam_target = {0.0f, 0.0f, 0.0f};
    Camera3_lookAt(&camera, cam_target);

    // for camera move controls
    vec3 cam_move_axes={0,0,0};
    vec3 cam_move_speed={0.1,0.1,0.1};

    // for camera rotation controls
    vec2 cam_rotate_axes={0,0};
    vec2 cam_rotate_speed={0.05,0.05};

    // Initialize 2D UI camera with aspect-ratio-aware AABB
    // Height is fixed at 100, width scales with window aspect ratio
    float ui_height = 100.0f;
    float ui_width = ui_height * ((float)window.size[0] / (float)window.size[1]);
    struct Camera2 camera_ui={};
    Camera2_create((vec4){0, 0, ui_width, ui_height}, &camera_ui);

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

                        const float new_aspect_ratio=window.size[0]/window.size[1];

                        // update 3d camera aspect ratio
                        camera.aspect_ratio=new_aspect_ratio;
                        Camera3_updateMatrices(&camera);

                        // Update UI camera AABB to maintain aspect ratio
                        float new_ui_width = ui_height * new_aspect_ratio;
                        Camera2_updateMatrices(&camera_ui, (vec4){0, 0, new_ui_width, ui_height});
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
                                    cam_move_axes[1]=-1;
                                }
                                break;
                            case XCB_KEYCODE_D:
                                {
                                    cam_move_axes[1]=1;
                                }
                                break;
                            case XCB_KEYCODE_LEFT:
                                {
                                    cam_rotate_axes[1]=1;
                                }
                                break;
                            case XCB_KEYCODE_RIGHT:
                                {
                                    cam_rotate_axes[1]=-1;
                                }
                                break;
                            case XCB_KEYCODE_UP:
                                {
                                    cam_rotate_axes[0]=1;
                                }
                                break;
                            case XCB_KEYCODE_DOWN:
                                {
                                    cam_rotate_axes[0]=-1;
                                }
                                break;
                            case XCB_KEYCODE_SPACE:
                                Camera3_lookAt(&camera,(vec3){0,0,0});
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
                            case XCB_KEYCODE_LEFT:
                                {
                                    cam_rotate_axes[1]=0;
                                }
                                break;
                            case XCB_KEYCODE_RIGHT:
                                {
                                    cam_rotate_axes[1]=0;
                                }
                                break;
                            case XCB_KEYCODE_UP:
                                {
                                    cam_rotate_axes[0]=0;
                                }
                                break;
                            case XCB_KEYCODE_DOWN:
                                {
                                    cam_rotate_axes[0]=0;
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
                case XCB_MOTION_NOTIFY:
                    {}
                    break;
                default:
                    printf("unhandled event %d\n",event_type);
            }
        }

        if(window_should_close) break;

        // move camera relative to its current orientation
        if(1){
            vec3 forward;
            quat_forward(camera.rotation, forward);
            vec3 forward_move;
            glm_vec3_scale(forward, cam_move_speed[0] * cam_move_axes[0], forward_move);
            glm_vec3_add(camera.pos, forward_move, camera.pos);

            vec3 right;
            quat_right(camera.rotation, right);
            vec3 right_move;
            glm_vec3_scale(right, cam_move_speed[1] * cam_move_axes[1], right_move);
            glm_vec3_add(camera.pos, right_move, camera.pos);
        }

        // rotate camera based on arrow key input
        if(cam_rotate_axes[0] != 0 || cam_rotate_axes[1] != 0){
            vec3 right;
            quat_right(camera.rotation, right);

            // Get camera's local axes for rotation
            vec3 up;
            quat_up(camera.rotation, up);
            quat_right(camera.rotation, right);

            // Create pitch rotation (around right axis)
            vec4 pitch_quat;
            if(cam_rotate_axes[0] != 0){
                glm_quatv(pitch_quat, cam_rotate_speed[0] * cam_rotate_axes[0], right);
            }else{
                glm_quat_identity(pitch_quat);
            }

            // Create yaw rotation (around up axis)
            vec4 yaw_quat;
            if(cam_rotate_axes[1] != 0){
                glm_quatv(yaw_quat, cam_rotate_speed[1] * cam_rotate_axes[1], up);
            }else{
                glm_quat_identity(yaw_quat);
            }

            // Apply yaw first, then pitch: combined_rotation = pitch * yaw * original_rotation
            vec4 yaw_pitch;
            glm_quat_mul(pitch_quat, yaw_quat, yaw_pitch);

            vec4 new_rotation;
            glm_quat_mul(yaw_pitch, camera.rotation, new_rotation);
            glm_quat_copy(new_rotation, camera.rotation);
        }

        Camera3_updateMatrices(&camera);

        Window_prepareDrawing(&window);

        Window_prepareDrawing3(&window);
            Object_draw3(&image_object, &camera);
            Object_draw3(&imported_object, &camera);

        Window_prepareDrawing2(&window);
            Object_draw2(&ui_object, &camera_ui);

        Window_finishDrawing(&window);
    }

    Window_destroy(&window);

    SystemInterface_destroy(&system_interface);

    return EXIT_SUCCESS;
}
