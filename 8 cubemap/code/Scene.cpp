// Este código es de dominio público
// angel.rodriguez@udit.es

#include "Scene.hpp"
#include <glm.hpp>                          
#include <gtc/matrix_transform.hpp>         
#include <gtc/type_ptr.hpp>                 
#include <SOIL2.h>
#include <string>
#include <iostream>
#include <vector>
#include <SDL3/SDL_keycode.h>

namespace udit
{
    // ==========================================
    // SHADERS ESCENA 3D (Terreno, Cubo)
    // ==========================================
    const std::string Scene::vertex_shader_code =
        "#version 330\n"
        "layout (location = 0) in vec3 a_position;\n"
        "layout (location = 1) in vec2 a_tex_coord;\n"
        "layout (location = 2) in vec3 a_normal;\n"
        "uniform mat4 u_model_view;\n"
        "uniform mat4 u_projection;\n"
        "out vec2 v_tex_coord;\n"
        "out vec3 v_normal;\n"
        "out vec3 v_frag_pos;\n"
        "void main() {\n"
        "    v_tex_coord = a_tex_coord;\n"
        "    // View Space normals para que la luz sea dinámica al mover objetos\n"
        "    v_normal = mat3(u_model_view) * a_normal;\n"
        "    v_frag_pos = vec3(u_model_view * vec4(a_position, 1.0));\n"
        "    gl_Position = u_projection * u_model_view * vec4(a_position, 1.0);\n"
        "}";

    const std::string Scene::fragment_shader_code =
        "#version 330\n"
        "uniform sampler2D u_texture;\n"
        "uniform vec3 u_light_dir;\n"
        "uniform vec3 u_light_color;\n"
        "uniform vec3 u_ambient_color;\n"
        "uniform float u_alpha; \n"
        "in vec2 v_tex_coord;\n"
        "in vec3 v_normal;\n"
        "in vec3 v_frag_pos;\n"
        "out vec4 f_color;\n"
        "void main() {\n"
        "    vec4 tex_color = texture(u_texture, v_tex_coord);\n"
        "    vec3 norm = normalize(v_normal);\n"
        "    vec3 light_dir = normalize(-u_light_dir);\n"
        "    float diff = max(dot(norm, light_dir), 0.0);\n"
        "    vec3 diffuse = diff * u_light_color;\n"
        "    vec3 ambient = u_ambient_color;\n"
        "    vec3 result = (ambient + diffuse) * tex_color.rgb;\n"
        "    f_color = vec4(result, tex_color.a * u_alpha);\n"
        "}";

    // ==========================================
    // SHADERS POSTPROCESO (Filtro Sepia)
    // ==========================================

    // Vertex Shader simple que cubre toda la pantalla (-1 a 1)
    const std::string Scene::post_vs_code =
        "#version 330 core\n"
        "layout (location = 0) in vec2 aPos;\n"
        "layout (location = 1) in vec2 aTexCoords;\n"
        "out vec2 TexCoords;\n"
        "void main() {\n"
        "    gl_Position = vec4(aPos.x, aPos.y, 0.0, 1.0);\n"
        "    TexCoords = aTexCoords;\n"
        "}";

    // Fragment Shader: SEPIA + VIÑETA
    const std::string Scene::post_fs_code =
        "#version 330 core\n"
        "out vec4 FragColor;\n"
        "in vec2 TexCoords;\n"
        "uniform sampler2D screenTexture;\n"
        "void main() {\n"
        "    // 1. Muestrear el color original\n"
        "    vec4 col = texture(screenTexture, TexCoords);\n"

        "    // 2. Aplicar filtro SEPIA\n"
        "    // Fórmula estándar para convertir RGB a Sepia\n"
        "    float r = dot(col.rgb, vec3(0.393, 0.769, 0.189));\n"
        "    float g = dot(col.rgb, vec3(0.349, 0.686, 0.168));\n"
        "    float b = dot(col.rgb, vec3(0.272, 0.534, 0.131));\n"
        "    vec3 sepia = vec3(r, g, b);\n"

        "    // 3. Efecto Viñeta (Bordes oscuros estilo foto antigua)\n"
        "    vec2 uv = TexCoords * (1.0 - TexCoords.yx);\n"
        "    float vig = uv.x * uv.y * 15.0;\n"
        "    vig = pow(vig, 0.25);\n"

        "    // Mezclamos sepia con viñeta\n"
        "    FragColor = vec4(sepia * vig, 1.0);\n"
        "}";


    // ==========================================
    // CONSTRUCTOR & INIT
    // ==========================================

    Scene::Scene(int width, int height)
        :
        skybox("../../../shared/assets/sky-cube-map-"),
        terrain("../../../shared/assets/height-map.png", 200.0f, 200.0f, 100, 100, 15.0f),
        cube(5.0f),
        width(width), height(height)
    {
        glEnable(GL_DEPTH_TEST);

        // Variables Cámara
        angle_around_x = 0.4f; angle_around_y = 0.0f;
        angle_delta_x = 0.0f;  angle_delta_y = 0.0f;
        pointer_pressed = false;
        camera.set_location(0.0f, 30.0f, 0.0f);

        move_forward = move_backward = move_left = move_right = move_up = move_down = false;
        camera_speed = 0.5f;

        // 1. Compilar Shaders 3D
        GLuint vs = glCreateShader(GL_VERTEX_SHADER);
        const char* vs_c = vertex_shader_code.c_str(); glShaderSource(vs, 1, &vs_c, nullptr); glCompileShader(vs);
        GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);
        const char* fs_c = fragment_shader_code.c_str(); glShaderSource(fs, 1, &fs_c, nullptr); glCompileShader(fs);

        program_id = glCreateProgram();
        glAttachShader(program_id, vs); glAttachShader(program_id, fs); glLinkProgram(program_id);

        model_view_matrix_id = glGetUniformLocation(program_id, "u_model_view");
        projection_matrix_id = glGetUniformLocation(program_id, "u_projection");
        glDeleteShader(vs); glDeleteShader(fs);

        // 2. Cargar Texturas
        there_is_texture = false;
        int w, h, c;
        unsigned char* px = SOIL_load_image("../../../shared/assets/ground.jpg", &w, &h, &c, SOIL_LOAD_RGBA);
        if (px) {
            glGenTextures(1, &texture_id);
            glBindTexture(GL_TEXTURE_2D, texture_id);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, px);
            glGenerateMipmap(GL_TEXTURE_2D);
            SOIL_free_image_data(px);
            there_is_texture = true;
        }

        unsigned char* stone_px = SOIL_load_image("../../../shared/assets/Stone.jpg", &w, &h, &c, SOIL_LOAD_RGBA);
        if (stone_px) {
            glGenTextures(1, &cube_texture_id);
            glBindTexture(GL_TEXTURE_2D, cube_texture_id);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, stone_px);
            glGenerateMipmap(GL_TEXTURE_2D);
            SOIL_free_image_data(stone_px);
        }
        else {
            cube_texture_id = texture_id;
        }

        // 3. Inicializar Sistema de Postproceso
        init_framebuffer(width, height);
        init_screen_quad();
        compile_postprocess_shader();

        // Ajustar vista inicial
        resize(width, height);
    }

    Scene::~Scene()
    {
        glDeleteFramebuffers(1, &fbo_id);
        glDeleteTextures(1, &fbo_texture_id);
        glDeleteRenderbuffers(1, &rbo_id);
    }

    // ==========================================
    // LOGICA DE POSTPROCESO
    // ==========================================

    void Scene::init_framebuffer(int w, int h)
    {
        // 1. Crear Framebuffer
        glGenFramebuffers(1, &fbo_id);
        glBindFramebuffer(GL_FRAMEBUFFER, fbo_id);

        // 2. Crear Textura donde pintaremos (Color attachment)
        glGenTextures(1, &fbo_texture_id);
        glBindTexture(GL_TEXTURE_2D, fbo_texture_id);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, w, h, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, fbo_texture_id, 0);

        // 3. Crear Renderbuffer para profundidad (Depth/Stencil attachment)
        glGenRenderbuffers(1, &rbo_id);
        glBindRenderbuffer(GL_RENDERBUFFER, rbo_id);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, w, h);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo_id);

        // 4. Comprobar que está completo
        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
            std::cerr << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << std::endl;

        // Desvincular para no pintar ahí por error todavía
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    void Scene::init_screen_quad()
    {
        // Rectángulo que cubre toda la pantalla (-1 a 1)
        float quadVertices[] = {
            // Pos        // TexCoords
            -1.0f,  1.0f,  0.0f, 1.0f,
            -1.0f, -1.0f,  0.0f, 0.0f,
             1.0f, -1.0f,  1.0f, 0.0f,

            -1.0f,  1.0f,  0.0f, 1.0f,
             1.0f, -1.0f,  1.0f, 0.0f,
             1.0f,  1.0f,  1.0f, 1.0f
        };

        glGenVertexArrays(1, &screen_vao_id);
        glGenBuffers(1, &screen_vbo_id);
        glBindVertexArray(screen_vao_id);
        glBindBuffer(GL_ARRAY_BUFFER, screen_vbo_id);
        glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);

        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
    }

    void Scene::compile_postprocess_shader()
    {
        GLuint vs = glCreateShader(GL_VERTEX_SHADER);
        const char* vs_c = post_vs_code.c_str(); glShaderSource(vs, 1, &vs_c, nullptr); glCompileShader(vs);

        GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);
        const char* fs_c = post_fs_code.c_str(); glShaderSource(fs, 1, &fs_c, nullptr); glCompileShader(fs);

        post_program_id = glCreateProgram();
        glAttachShader(post_program_id, vs);
        glAttachShader(post_program_id, fs);
        glLinkProgram(post_program_id);

        glDeleteShader(vs); glDeleteShader(fs);
    }


    // ==========================================
    // UPDATE & RENDER
    // ==========================================

    void Scene::update()
    {
        // Rotación Cámara
        angle_around_x += angle_delta_x;
        angle_around_y += angle_delta_y;
        if (angle_around_x < -1.5f) angle_around_x = -1.5f; else if (angle_around_x > 1.5f) angle_around_x = 1.5f;
        angle_delta_x = 0; angle_delta_y = 0;

        glm::mat4 cam_rot(1);
        cam_rot = glm::rotate(cam_rot, angle_around_y, glm::vec3(0, 1, 0));
        cam_rot = glm::rotate(cam_rot, angle_around_x, glm::vec3(1, 0, 0));

        auto loc = camera.get_location();
        camera.set_target(loc.x, loc.y, loc.z - 1.0f);
        camera.rotate(cam_rot);

        // Movimiento
        glm::vec3 f = glm::normalize(glm::vec3(camera.get_target()) - glm::vec3(camera.get_location()));
        glm::vec3 r = glm::normalize(glm::cross(f, glm::vec3(0, 1, 0)));
        glm::vec3 m(0);
        if (move_forward) m += f; if (move_backward) m -= f;
        if (move_right) m += r; if (move_left) m -= r;
        if (move_up) m += glm::vec3(0, 1, 0); if (move_down) m -= glm::vec3(0, 1, 0);
        if (glm::length(m) > 0) camera.move(m * camera_speed);

        cube_angle += 0.01f;
    }

    void Scene::render()
    {
        // ----------------------------------------------------
        // PASE 1: PINTAR LA ESCENA EN EL FRAMEBUFFER (OFF-SCREEN)
        // ----------------------------------------------------
        glBindFramebuffer(GL_FRAMEBUFFER, fbo_id);

        // Habilitamos Depth Test para el 3D
        glEnable(GL_DEPTH_TEST);
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // --- Render Skybox ---
        glDepthMask(GL_FALSE);
        skybox.render(camera);
        glDepthMask(GL_TRUE);

        // --- Render Objetos 3D ---
        glUseProgram(program_id);

        glm::mat4 view = camera.get_transform_matrix_inverse();
        glm::mat4 proj = camera.get_projection_matrix();

        glm::vec3 light_dir_world = glm::vec3(0.5f, -1.0f, 0.5f);
        glm::vec3 light_dir_view = glm::vec3(view * glm::vec4(light_dir_world, 0.0f));

        glUniform3fv(glGetUniformLocation(program_id, "u_light_dir"), 1, glm::value_ptr(light_dir_view));
        glUniform3f(glGetUniformLocation(program_id, "u_light_color"), 1.0f, 0.95f, 0.9f);
        glUniform3f(glGetUniformLocation(program_id, "u_ambient_color"), 0.2f, 0.2f, 0.3f);
        glUniformMatrix4fv(projection_matrix_id, 1, GL_FALSE, glm::value_ptr(proj));

        // Terreno
        glUniform1f(glGetUniformLocation(program_id, "u_alpha"), 1.0f);
        glUniformMatrix4fv(model_view_matrix_id, 1, GL_FALSE, glm::value_ptr(view));
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture_id);
        terrain.render();

        // Cubo (Transparente)
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glUniform1f(glGetUniformLocation(program_id, "u_alpha"), 0.75f);
        glm::mat4 model_cube(1.0f);
        model_cube = glm::translate(model_cube, glm::vec3(0.0f, 40.0f, 0.0f)); // Altura 40
        model_cube = glm::rotate(model_cube, cube_angle, glm::vec3(1.0f, 1.0f, 0.0f));
        model_cube = glm::scale(model_cube, glm::vec3(4.0f, 4.0f, 4.0f)); // Escala x4
        glm::mat4 model_view_cube = view * model_cube;
        glUniformMatrix4fv(model_view_matrix_id, 1, GL_FALSE, glm::value_ptr(model_view_cube));
        glBindTexture(GL_TEXTURE_2D, cube_texture_id);
        cube.render();
        glDisable(GL_BLEND);

        // ----------------------------------------------------
        // PASE 2: PINTAR EL QUAD EN LA PANTALLA CON EFECTOS
        // ----------------------------------------------------
        glBindFramebuffer(GL_FRAMEBUFFER, 0); // Volvemos a la pantalla normal
        glDisable(GL_DEPTH_TEST); // No necesitamos profundidad para una pantalla plana
        glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        glUseProgram(post_program_id);
        glBindVertexArray(screen_vao_id);

        // Usamos la textura que acabamos de pintar
        glBindTexture(GL_TEXTURE_2D, fbo_texture_id);

        glDrawArrays(GL_TRIANGLES, 0, 6);
    }

    void Scene::resize(int w, int h)
    {
        width = w; height = h;
        camera.set_ratio((float)w / h);
        glViewport(0, 0, w, h);

        // IMPORTANTE: Al cambiar tamaño ventana, debemos redimensionar el framebuffer
        glBindTexture(GL_TEXTURE_2D, fbo_texture_id);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, w, h, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);

        glBindRenderbuffer(GL_RENDERBUFFER, rbo_id);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, w, h);
    }

    void Scene::on_drag(float x, float y) { if (pointer_pressed) { angle_delta_x = 1.025f * (last_pointer_y - y) / height; angle_delta_y = 1.025f * (last_pointer_x - x) / width; last_pointer_x = x; last_pointer_y = y; } }
    void Scene::on_click(float x, float y, bool d) { last_pointer_x = x; last_pointer_y = y; pointer_pressed = d; }
    void Scene::on_key(int k, bool p) {
        switch (k) {
        case 'w':case 'W': move_forward = p; break; case 's':case 'S': move_backward = p; break;
        case 'a':case 'A': move_left = p; break;    case 'd':case 'D': move_right = p; break;
        case 'e':case 'E': move_up = p; break;      case 'q':case 'Q': move_down = p; break;
        }
    }
}