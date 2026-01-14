// Cube.hpp
// angel.rodriguez@udit.es

#ifndef CUBE_HEADER
#define CUBE_HEADER

#include <glad/gl.h>
#include <vector>

namespace udit
{
    class Cube
    {
    private:
        // ID del "Vertex Array Object". Es el 'archivador' que guarda la configuración
        // de cómo están organizados los vértices en la memoria de la tarjeta gráfica.
        GLuint vao_id;

        // IDs de los "Vertex Buffer Objects". Son los buffers de datos crudos.
        // Usamos 3: uno para posiciones (0), uno para coordenadas de textura (1) y otro para normales (2).
        GLuint vbo_ids[3];

        // Cantidad total de vértices a dibujar (36 para un cubo hecho de triángulos).
        GLsizei vertex_count;

    public:
        // Constructor: Recibe el tamaño (lado) del cubo
        Cube(float size);

        // Destructor: Limpia la memoria de la gráfica al borrar el objeto
        ~Cube();

        // Función que manda la orden de dibujo a OpenGL
        void render();
    };
}

#endif