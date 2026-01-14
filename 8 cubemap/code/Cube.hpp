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
        GLuint vao_id;
        GLuint vbo_ids[3]; // 0: Pos, 1: UV, 2: Normal
        GLsizei vertex_count;

    public:
        Cube(float size);
        ~Cube();
        void render();
    };
}

#endif