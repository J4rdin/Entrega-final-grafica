// Cube.cpp
// angel.rodriguez@udit.es

#include "Cube.hpp"
#include <vector>

namespace udit
{
    Cube::Cube(float size)
    {
        float s = size * 0.5f;

        // Definimos los vertices: Pos(3) + UV(2) + Normal(3)
        // 36 vértices en total (6 caras * 2 triángulos * 3 vértices)
        float vertices[] = {
            // Front face (Normal +Z)
            -s, -s,  s,  0, 0,  0, 0, 1,
             s, -s,  s,  1, 0,  0, 0, 1,
             s,  s,  s,  1, 1,  0, 0, 1,
            -s, -s,  s,  0, 0,  0, 0, 1,
             s,  s,  s,  1, 1,  0, 0, 1,
            -s,  s,  s,  0, 1,  0, 0, 1,

            // Back face (Normal -Z)
             s, -s, -s,  0, 0,  0, 0,-1,
            -s, -s, -s,  1, 0,  0, 0,-1,
            -s,  s, -s,  1, 1,  0, 0,-1,
             s, -s, -s,  0, 0,  0, 0,-1,
            -s,  s, -s,  1, 1,  0, 0,-1,
             s,  s, -s,  0, 1,  0, 0,-1,

             // Left face (Normal -X)
             -s, -s, -s,  0, 0, -1, 0, 0,
             -s, -s,  s,  1, 0, -1, 0, 0,
             -s,  s,  s,  1, 1, -1, 0, 0,
             -s, -s, -s,  0, 0, -1, 0, 0,
             -s,  s,  s,  1, 1, -1, 0, 0,
             -s,  s, -s,  0, 1, -1, 0, 0,

             // Right face (Normal +X)
              s, -s,  s,  0, 0,  1, 0, 0,
              s, -s, -s,  1, 0,  1, 0, 0,
              s,  s, -s,  1, 1,  1, 0, 0,
              s, -s,  s,  0, 0,  1, 0, 0,
              s,  s, -s,  1, 1,  1, 0, 0,
              s,  s,  s,  0, 1,  1, 0, 0,

              // Top face (Normal +Y)
              -s,  s,  s,  0, 0,  0, 1, 0,
               s,  s,  s,  1, 0,  0, 1, 0,
               s,  s, -s,  1, 1,  0, 1, 0,
              -s,  s,  s,  0, 0,  0, 1, 0,
               s,  s, -s,  1, 1,  0, 1, 0,
              -s,  s, -s,  0, 1,  0, 1, 0,

              // Bottom face (Normal -Y)
              -s, -s, -s,  0, 0,  0,-1, 0,
               s, -s, -s,  1, 0,  0,-1, 0,
               s, -s,  s,  1, 1,  0,-1, 0,
              -s, -s, -s,  0, 0,  0,-1, 0,
               s, -s,  s,  1, 1,  0,-1, 0,
              -s, -s,  s,  0, 1,  0,-1, 0,
        };

        vertex_count = 36;

        // Separamos los datos en vectores para los VBOs
        std::vector<float> pos, uvs, norm;
        for (int i = 0; i < 36; ++i) {
            int base = i * 8;
            pos.push_back(vertices[base + 0]); pos.push_back(vertices[base + 1]); pos.push_back(vertices[base + 2]);
            uvs.push_back(vertices[base + 3]); uvs.push_back(vertices[base + 4]);
            norm.push_back(vertices[base + 5]); norm.push_back(vertices[base + 6]); norm.push_back(vertices[base + 7]);
        }

        glGenVertexArrays(1, &vao_id);
        glGenBuffers(3, vbo_ids);

        glBindVertexArray(vao_id);

        // Position
        glBindBuffer(GL_ARRAY_BUFFER, vbo_ids[0]);
        glBufferData(GL_ARRAY_BUFFER, pos.size() * sizeof(float), pos.data(), GL_STATIC_DRAW);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
        glEnableVertexAttribArray(0);

        // UVs
        glBindBuffer(GL_ARRAY_BUFFER, vbo_ids[1]);
        glBufferData(GL_ARRAY_BUFFER, uvs.size() * sizeof(float), uvs.data(), GL_STATIC_DRAW);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, 0);
        glEnableVertexAttribArray(1);

        // Normals
        glBindBuffer(GL_ARRAY_BUFFER, vbo_ids[2]);
        glBufferData(GL_ARRAY_BUFFER, norm.size() * sizeof(float), norm.data(), GL_STATIC_DRAW);
        glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, 0);
        glEnableVertexAttribArray(2);
    }

    Cube::~Cube() {
        glDeleteVertexArrays(1, &vao_id);
        glDeleteBuffers(3, vbo_ids);
    }

    void Cube::render() {
        glBindVertexArray(vao_id);
        glDrawArrays(GL_TRIANGLES, 0, vertex_count);
    }
}