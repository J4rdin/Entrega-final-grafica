// Scene.hpp
// angel.rodriguez@udit.es

#ifndef SCENE_HEADER
#define SCENE_HEADER

#include "Camera.hpp"
#include "Skybox.hpp"
#include "Terrain.hpp"
#include "Cube.hpp"
#include <map>

namespace udit
{
    class Scene
    {
    private:
        Camera camera;
        Skybox skybox;
        Terrain terrain;
        Cube cube;

        int    width;
        int    height;

        // Variables Cámara
        float  angle_around_x, angle_around_y;
        float  angle_delta_x, angle_delta_y;
        bool   pointer_pressed;
        float  last_pointer_x, last_pointer_y;

        // Shaders Escena (3D)
        static const std::string vertex_shader_code;
        static const std::string fragment_shader_code;
        GLuint  program_id;
        GLint   model_view_matrix_id, projection_matrix_id;

        // Texturas
        GLuint  texture_id;
        GLuint  cube_texture_id;
        bool    there_is_texture;

        // Animación
        float cube_angle = 0.0f;

        // Movimiento
        bool move_forward, move_backward, move_left, move_right, move_up, move_down;
        float camera_speed;

        // --- VARIABLES NUEVAS PARA POSTPROCESO ---
        GLuint fbo_id;         // Framebuffer Object
        GLuint fbo_texture_id; // La textura donde pintamos la escena
        GLuint rbo_id;         // Renderbuffer (para la profundidad)

        GLuint screen_vao_id;  // El rectángulo de pantalla
        GLuint screen_vbo_id;

        GLuint post_program_id; // Shader de postproceso
        static const std::string post_vs_code; // Vertex shader postproceso
        static const std::string post_fs_code; // Fragment shader postproceso

    public:
        Scene(int width, int height);
        ~Scene(); // Importante para limpiar el FBO

        void update();
        void render();
        void resize(int width, int height);
        void on_drag(float pointer_x, float pointer_y);
        void on_click(float pointer_x, float pointer_y, bool down);
        void on_key(int key_code, bool pressed);

    private:
        void init_framebuffer(int width, int height);
        void init_screen_quad();
        void compile_postprocess_shader();
    };
}
#endif