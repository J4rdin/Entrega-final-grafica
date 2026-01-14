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
        // --- ELEMENTOS 3D ---
        Camera camera;    // Gestiona la vista y la proyeccion (perspectiva)
        Skybox skybox;    // El cubo de fondo (cielo)
        Terrain terrain;  // La malla del suelo generada por heightmap
        Cube cube;        // El cubo flotante

        // Dimensiones de la ventana (para ajustar el viewport y texturas)
        int    width;
        int    height;

        // --- CONTROL DE CÁMARA (RATÓN) ---
        float  angle_around_x, angle_around_y; // Angulos actuales de la cámara
        float  angle_delta_x, angle_delta_y;   // Cuánto se ha movido el ratón en este frame
        bool   pointer_pressed;                // ¿Esta el botón del raton pulsado?
        float  last_pointer_x, last_pointer_y; // Última posicion conocida del cursor

        // --- SHADERS PRINCIPALES (GEOMETRIA 3D) ---
        // Código fuente GLSL y los IDs del programa compilado
        static const std::string vertex_shader_code;
        static const std::string fragment_shader_code;
        GLuint  program_id;
        // Localizaciones de las variables 'uniform' para enviar matrices al shader
        GLint   model_view_matrix_id, projection_matrix_id;

        // --- TEXTURAS ---
        GLuint  texture_id;       // ID de la textura del suelo
        GLuint  cube_texture_id;  // ID de la textura del cubo
        bool    there_is_texture; // Flag de control

        // --- ANIMACIÓN ---
        float cube_angle = 0.0f; // Angulo de rotación del cubo (se incrementa en update)

        // --- CONTROL DE CAMARA (TECLADO) ---
        // Flags para saber qué teclas (WASD + EQ) estan pulsadas
        bool move_forward, move_backward, move_left, move_right, move_up, move_down;
        float camera_speed; // Velocidad de desplazamiento

        // --- VARIABLES PARA POST-PROCESO (Filtros de pantalla) ---
        GLuint fbo_id;         // Framebuffer Object: Memoria donde dibujamos "off-screen"
        GLuint fbo_texture_id; // La textura resultante del primer pase de renderizado
        GLuint rbo_id;         // Renderbuffer: Necesario para guardar la profundidad (Z-Buffer) en el FBO

        GLuint screen_vao_id;  // VAO del cuadrado plano que cubre la pantalla
        GLuint screen_vbo_id;  // VBO del cuadrado

        GLuint post_program_id; // Programa de Shader para el efecto final (Sepia/Viñeta)
        static const std::string post_vs_code; // Vertex shader del post-proceso
        static const std::string post_fs_code; // Fragment shader del post-proceso

    public:
        // Constructor: Inicializa todo (shaders, buffers, carga archivos...)
        Scene(int width, int height);

        // Destructor: Limpia memoria de OpenGL (buffers, texturas) al cerrar
        ~Scene();

        // Actualiza la logica: movimiento de camara, animaciones, fisica...
        void update();

        // Dibuja la escena. Aqui ocurre el renderizado en 2 pasos (Off-screen -> Pantalla)
        void render();

        // Se llama cuando cambia el tamaño de la ventana para reajustar texturas y cámara
        void resize(int width, int height);

        // --- GESTION DE EVENTOS (Input) ---
        void on_drag(float pointer_x, float pointer_y);             // Movimiento del raton
        void on_click(float pointer_x, float pointer_y, bool down); // Clics
        void on_key(int key_code, bool pressed);                    // Pulsación de teclas

    private:
        // Funciones auxiliares internas para configurar el Post-Proceso
        void init_framebuffer(int width, int height); // Crea el FBO y texturas asociadas
        void init_screen_quad();                      // Crea la geometría del cuadrado de pantalla completa
        void compile_postprocess_shader();            // Compila los shaders de efectos visuales
    };
}
#endif