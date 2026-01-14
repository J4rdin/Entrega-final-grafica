// Terrain.hpp
// angel.rodriguez@udit.es

#ifndef GROUND_HEADER
#define GROUND_HEADER

#include <glad/gl.h>
#include <string>
#include <vector>

namespace udit
{

    class Terrain
    {
    private:

        enum
        {
            COORDINATES_VBO,
            TEXTURE_UVS_VBO,
            NORMALS_VBO,      // <--- NUEVO: Buffer para las normales
            INDICES_EBO,
            VBO_COUNT
        };

    private:

        GLuint  vao_id;
        GLuint  vbo_ids[VBO_COUNT];
        GLsizei number_of_indices;

    public:

        Terrain(const std::string& heightmap_path, float width, float depth, unsigned x_slices, unsigned z_slices, float max_height);
        ~Terrain();

    public:

        void render();

    };

}

#endif