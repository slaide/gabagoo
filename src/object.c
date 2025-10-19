#include<limits.h>
#include<stdlib.h>
#include<stdio.h>
#include<string.h>

#include<object.h>
#include<window.h>
#include<util.h>

#include<xcb/xcb.h>
#include<glad/egl.h>
#include<glad/gl.h>

const char*fileContents(const char*filepath){
    FILE* f=fopen(filepath,"rb");
    CHECK(f!=nullptr,"failed to open file");
    fseek(f,0,SEEK_END);
    int filelen=ftell(f);
    fseek(f,0,SEEK_SET);
    char*content=calloc(1,filelen+1);
    fread(content,filelen,1,f);
    fclose(f);
    return content;
}

enum SYSTEM_RESULT SystemInterface_create(struct SystemInterface*system_interface){
    system_interface->con=xcb_connect(nullptr,nullptr);
    CHECK(system_interface->con != nullptr,"xcb connection failed");

    return SYSTEM_RESULT_SUCCESS;
}
void SystemInterface_destroy(struct SystemInterface*system_interface){
    xcb_disconnect(system_interface->con);
}

void Material_create(const char*vertexShaderPath,const char*fragmentShaderPath,struct Material*material){
    const char*vertexShaderSource=fileContents(vertexShaderPath);
    uint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource,nullptr);
    glCompileShader(vertexShader);
    //free((void*)vertexShaderSource);

    const char*fragmentShaderSource=fileContents(fragmentShaderPath);
    uint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource,nullptr);
    glCompileShader(fragmentShader);
    //free((void*)fragmentShaderSource);

    uint shaderProgram=glCreateProgram();
    glAttachShader(shaderProgram,fragmentShader);
    glAttachShader(shaderProgram,vertexShader);
    glLinkProgram(shaderProgram);

    GLint
        view_loc=glGetUniformLocation(shaderProgram,"view"),
        proj_loc=glGetUniformLocation(shaderProgram,"projection"),
        model_loc=glGetUniformLocation(shaderProgram,"model");

    *material=(struct Material){
        .vertexShader=vertexShader,
        .fragmentShader=fragmentShader,
        .shaderProgram=shaderProgram,
        
        .view_loc=view_loc,
        .proj_loc=proj_loc,
        .model_loc=model_loc
    };
}

void Mesh_create(
    int num_faces,
    uint*faces,

    struct VertexInformation *vertex_info,

    struct Mesh*mesh
){

    // vertex array object (contains pointers to vertex buffers)
    uint vao;
    glGenVertexArrays(1,&vao);

    glBindVertexArray(vao);

    // vertex buffer object
    uint vbo;
    glGenBuffers(1,&vbo);

    // vertex buffer has type array buffer
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    // upload data
    // STATIC_DRAW: written once, read many times
    glBufferData(GL_ARRAY_BUFFER, vertex_info->num_vertices*vertex_info->stride, vertex_info->vertex_data, GL_STATIC_DRAW);

    // element buffer object
    uint ebo;
    glGenBuffers(1,&ebo);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, num_faces*3*sizeof(uint), faces, GL_STATIC_DRAW);

    for(int i=0;i<vertex_info->numVertexAttributes;i++){
        const struct VertexAttribute attribute=vertex_info->vertexAttributes[i];
        glVertexAttribPointer(
            attribute.location,
            attribute.numVertexItems,attribute.itemType,
            GL_FALSE,
            vertex_info->stride,
            (void*)attribute.itemOffset
        );
        glEnableVertexAttribArray(attribute.location);
    }

    *mesh=(struct Mesh){
        .vao=vao,
        .vbo=vbo,
        .ebo=ebo,
        .num_faces=num_faces,
        .num_vertices=vertex_info->num_vertices
    };
}

void Mesh_parseObj(const char*path,struct Mesh*mesh){
    const char*filecontents=fileContents(path);
    const int len=strlen(filecontents);

    int num_v=0;
    float*v=nullptr;
    int num_vn=0;
    float*vn=nullptr;
    int num_vt=0;
    float*vt=nullptr;

    int num_vertices=0;
    float *vertices=nullptr;

    int num_faces=0;
    uint *faces=nullptr;

    char*objectname=nullptr;
    char*groupname=nullptr;
    discard groupname;

    const char*p=filecontents;
    for(;p<filecontents+len;){
        switch(p[0]){
            case ' ':
            case '\n':
                p++;
                continue;

            case 'o':
                {
                    p+=2;
                    const char*namestart=p;
                    while(p[0]!='\n')p++;
                    objectname=malloc(p-namestart+1);
                    memcpy(objectname,namestart,p-namestart);
                    printf("object name '%.*s'\n",(int)(p-namestart),objectname);
                }
                break;

            case 'v':
                switch(p[1]){
                    case ' ':
                        {
                            p+=2;

                            float vert[4]={};
                            int i=0;
                            for(;i<4;i++){
                                char*end=(char*)filecontents+len;

                                vert[i]=strtof(p, &end);
                                if(p==end)break;
                                p=end;
                            }
                            
                            num_v++;
                            v=realloc(v,num_v*3*sizeof(float));
                            v[(num_v-1)*3+0]=vert[0];
                            v[(num_v-1)*3+1]=vert[1];
                            v[(num_v-1)*3+2]=vert[2];
                        }
                        break;
                    case 'n':
                        {
                            p+=3;

                            float vert[4]={};
                            int i=0;
                            for(;i<4;i++){
                                char*end=(char*)filecontents+len;

                                vert[i]=strtof(p, &end);
                                if(p==end)break;
                                p=end;
                            }
                            
                            num_vn++;
                            vn=realloc(vn,num_vn*3*sizeof(float));
                            vn[(num_vn-1)*3+0]=vert[0];
                            vn[(num_vn-1)*3+1]=vert[1];
                            vn[(num_vn-1)*3+2]=vert[2];
                        }
                        break;
                    case 't':
                        {
                            p+=3;

                            float vert[4]={};
                            int i=0;
                            for(;i<4;i++){
                                char*end=(char*)filecontents+len;

                                vert[i]=strtof(p, &end);
                                if(p==end)break;
                                p=end;
                            }
                            
                            num_vt++;
                            vt=realloc(vt,num_vt*2*sizeof(float));
                            vt[(num_vt-1)*2+0]=vert[0];
                            vt[(num_vt-1)*2+1]=vert[1];
                        }
                        break;
                }
                break;

            case 'f':
                {
                    p+=2;

                    uint newfaces[4][3]={
                        {-1,-1,-1},
                        {-1,-1,-1},
                        {-1,-1,-1},
                        {-1,-1,-1}
                    };

                    int i=0;
                    for(;i<4;i++){
                        char*end=(char*)filecontents+len;

                        for(int j=0;j<3;j++){
                            uint val=strtod(p,&end);
                            // obj is 1-indexed
                            newfaces[i][j]=val-1;

                            p=end;

                            if(p[0]=='/'){
                                p++;
                                continue;
                            }

                            break;
                        }

                        if(newfaces[i][0]==(uint)-1)break;

                        // Add new vertex into 'vertices' (3 floats pos, 2 floats vt, 3 floats vn, see 'vertex_info')
                        vertices=realloc(vertices,(num_vertices+1)*8*sizeof(float));

                        // Position data (3 floats)
                        if(newfaces[i][0]==(uint)-1){
                            vertices[num_vertices*8+0]=0.0f;
                            vertices[num_vertices*8+1]=0.0f;
                            vertices[num_vertices*8+2]=0.0f;
                        }else{
                            for(int k=0;k<3;k++){
                                int v_idx=newfaces[i][0]*3+k;
                                vertices[num_vertices*8+k]=v[v_idx];
                            }
                        }

                        // Texture coordinate data (2 floats)
                        if(newfaces[i][1]==(uint)-1){
                            vertices[num_vertices*8+3]=0.0f;
                            vertices[num_vertices*8+4]=0.0f;
                        }else{
                            for(int k=0;k<2;k++){
                                int vt_idx=newfaces[i][1]*2+k;
                                vertices[num_vertices*8+3+k]=vt[vt_idx];
                            }
                        }

                        // Normal data (3 floats)
                        if(newfaces[i][2]==(uint)-1){
                            vertices[num_vertices*8+5]=0.0f;
                            vertices[num_vertices*8+6]=0.0f;
                            vertices[num_vertices*8+7]=0.0f;
                        }else{
                            for(int k=0;k<3;k++){
                                int vn_idx=newfaces[i][2]*3+k;
                                vertices[num_vertices*8+5+k]=vn[vn_idx];
                            }
                        }

                        num_vertices+=1;
                    }

                    // Add face indices for the parsed face
                    if(i==3){
                        // Triangle: single face
                        faces=realloc(faces,(num_faces+1)*3*sizeof(uint));
                        faces[num_faces*3+0]=num_vertices-3;
                        faces[num_faces*3+1]=num_vertices-2;
                        faces[num_faces*3+2]=num_vertices-1;
                        num_faces+=1;
                    }else if(i==4){
                        // Quad: triangulate into two faces (0,1,2) and (0,2,3)
                        faces=realloc(faces,(num_faces+2)*3*sizeof(uint));

                        // First triangle
                        faces[num_faces*3+0]=num_vertices-4;
                        faces[num_faces*3+1]=num_vertices-3;
                        faces[num_faces*3+2]=num_vertices-2;

                        // Second triangle
                        faces[(num_faces+1)*3+0]=num_vertices-4;
                        faces[(num_faces+1)*3+1]=num_vertices-2;
                        faces[(num_faces+1)*3+2]=num_vertices-1;

                        num_faces+=2;
                    }
                }
                break;

            case 's':
                {
                    while(p[0]!='\n')p++;
                }
                break;
            
            case '#':
                // comment -> skip to end of line, then over eol
                while(*p!='\n')p++;
                p++;
                continue; 

            default:CHECK(false,"unknown char %c\nleft\n%s\n",p[0],p);
        }
    }

    struct VertexAttribute vertex_attributes[3]={
        (struct VertexAttribute){
            .numVertexItems=3,
            .itemType=GL_FLOAT,
            .itemOffset=0,
            .location=0
        },
        (struct VertexAttribute){
            .numVertexItems=2,
            .itemType=GL_FLOAT,
            .itemOffset=3*sizeof(float),
            .location=1
        },
        (struct VertexAttribute){
            .numVertexItems=3,
            .itemType=GL_FLOAT,
            .itemOffset=5*sizeof(float),
            .location=2
        },
    };

    struct VertexInformation vertex_info={
        .num_vertices=num_vertices,
        .vertex_data=vertices,
        .numVertexAttributes=3,
        .vertexAttributes=vertex_attributes,
        .stride=(3+2+3)*sizeof(float)
    };
    Mesh_create(num_faces,faces,&vertex_info,mesh);

    printf("mesh had %d vertices %d faces\n",vertex_info.num_vertices,num_faces);
}

void Transform_getModelMatrix(struct Transform*transform,mat4*object_matrix){
    mat4 rotation_matrix;

    // Initialize to identity
    glm_mat4_identity(*object_matrix);

    // Apply translation
    glm_translate(*object_matrix, transform->pos);

    // Apply rotation (convert quaternion to rotation matrix)
    glm_quat_mat4(transform->rot, rotation_matrix);
    glm_mat4_mul(*object_matrix, rotation_matrix, *object_matrix);

    // Apply scale
    glm_scale(*object_matrix, transform->scale);
}

void Object_create(
    struct Mesh*mesh,
    struct Material*material,

    struct Object*object
){
    *object=(struct Object){
        .transform={
            .scale[0]=1,
            .scale[1]=1,
            .scale[2]=1,
        },
        .mesh=mesh,
        .material=material,

        .num_children=0,
        .children=nullptr
    };

    Object_updateTransformMatrix(object);
}
void Object_updateTransformMatrix(struct Object*object){
    mat4 obj_mat;
    Transform_getModelMatrix(&object->transform,&obj_mat);
    
    glUseProgram(object->material->shaderProgram);
    glUniformMatrix4fv(object->material->model_loc,1,GL_FALSE,(float*)obj_mat);
}
void Object_appendChild(struct Object*child,struct Object*parent){
    parent->num_children++;
    parent->children=realloc(parent->children,parent->num_children*sizeof(struct Object*));
    parent->children[parent->num_children-1]=child;
}

void Object_draw(struct Object*object){
    glUseProgram(object->material->shaderProgram);

    glBindVertexArray(object->mesh->vao);
    glDrawElements(GL_TRIANGLES,object->mesh->num_faces*3,GL_UNSIGNED_INT,0);

    for(int c=0;c<object->num_children;c++){
        Object_draw(object->children[c]);
    }
}

void Object_draw3(struct Object*object, struct Camera3*camera){
    glUseProgram(object->material->shaderProgram);
    glUniformMatrix4fv(object->material->view_loc,1,GL_FALSE,(float*)camera->view_mat);
    glUniformMatrix4fv(object->material->proj_loc,1,GL_FALSE,(float*)camera->proj_mat);
    Object_draw(object);
}

void Object_draw2(struct Object*object, struct Camera2*camera){
    glUseProgram(object->material->shaderProgram);
    glUniformMatrix4fv(object->material->view_loc,1,GL_FALSE,(float*)camera->view_mat);
    glUniformMatrix4fv(object->material->proj_loc,1,GL_FALSE,(float*)camera->proj_mat);
    Object_draw(object);
}

void Object_destroy(struct Object*object){
    free(object->children);
}
