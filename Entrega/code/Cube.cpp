// Cube.cpp
// angel.rodriguez@udit.es

#include "Cube.hpp"
#include <vector>

namespace udit
{
    Cube::Cube(float size)
    {
        float s = size * 0.5f;

        // Cada vértice tiene 8 números: X, Y, Z (Posición) | U, V (Textura) | NX, NY, NZ (Normal)
        float vertices[] = {
            // --- CARA FRONTAL (Apunta hacia +Z) ---
            // Posición(xyz) | Textura(uv) | Normal(xyz)
            -s, -s,  s,  0, 0,  0, 0, 1, // Abajo-Izq
             s, -s,  s,  1, 0,  0, 0, 1, // Abajo-Der
             s,  s,  s,  1, 1,  0, 0, 1, // Arriba-Der
            -s, -s,  s,  0, 0,  0, 0, 1, // Abajo-Izq (Repetido para cerrar triángulo)
             s,  s,  s,  1, 1,  0, 0, 1, // Arriba-Der
            -s,  s,  s,  0, 1,  0, 0, 1, // Arriba-Izq

            // --- CARA TRASERA (Apunta hacia -Z) ---
             s, -s, -s,  0, 0,  0, 0,-1,
            -s, -s, -s,  1, 0,  0, 0,-1,
            -s,  s, -s,  1, 1,  0, 0,-1,
             s, -s, -s,  0, 0,  0, 0,-1,
            -s,  s, -s,  1, 1,  0, 0,-1,
             s,  s, -s,  0, 1,  0, 0,-1,

             // --- CARA IZQUIERDA (Apunta hacia -X) ---
             -s, -s, -s,  0, 0, -1, 0, 0,
             -s, -s,  s,  1, 0, -1, 0, 0,
             -s,  s,  s,  1, 1, -1, 0, 0,
             -s, -s, -s,  0, 0, -1, 0, 0,
             -s,  s,  s,  1, 1, -1, 0, 0,
             -s,  s, -s,  0, 1, -1, 0, 0,

             // --- CARA DERECHA (Apunta hacia +X) ---
              s, -s,  s,  0, 0,  1, 0, 0,
              s, -s, -s,  1, 0,  1, 0, 0,
              s,  s, -s,  1, 1,  1, 0, 0,
              s, -s,  s,  0, 0,  1, 0, 0,
              s,  s, -s,  1, 1,  1, 0, 0,
              s,  s,  s,  0, 1,  1, 0, 0,

              // --- CARA SUPERIOR (Apunta hacia +Y) ---
              -s,  s,  s,  0, 0,  0, 1, 0,
               s,  s,  s,  1, 0,  0, 1, 0,
               s,  s, -s,  1, 1,  0, 1, 0,
              -s,  s,  s,  0, 0,  0, 1, 0,
               s,  s, -s,  1, 1,  0, 1, 0,
              -s,  s, -s,  0, 1,  0, 1, 0,

              // --- CARA INFERIOR (Apunta hacia -Y) ---
              -s, -s, -s,  0, 0,  0,-1, 0,
               s, -s, -s,  1, 0,  0,-1, 0,
               s, -s,  s,  1, 1,  0,-1, 0,
              -s, -s, -s,  0, 0,  0,-1, 0,
               s, -s,  s,  1, 1,  0,-1, 0,
              -s, -s,  s,  0, 1,  0,-1, 0,
        };

        vertex_count = 36; // 6 caras * 2 triángulos * 3 vértices

        // Separamos el array gigante 'vertices' en 3 vectores organizados
        // Esto facilita enviarlos a OpenGL por separado.
        std::vector<float> pos, uvs, norm;
        for (int i = 0; i < 36; ++i) {
            int base = i * 8; // Cada vértice ocupa 8 huecos en el array original
            // 3 floats para posición
            pos.push_back(vertices[base + 0]); pos.push_back(vertices[base + 1]); pos.push_back(vertices[base + 2]);
            // 2 floats para UVs
            uvs.push_back(vertices[base + 3]); uvs.push_back(vertices[base + 4]);
            // 3 floats para normales
            norm.push_back(vertices[base + 5]); norm.push_back(vertices[base + 6]); norm.push_back(vertices[base + 7]);
        }

        // Generamos los identificadores de OpenGL
        glGenVertexArrays(1, &vao_id);
        glGenBuffers(3, vbo_ids);

        // Activamos el VAO para empezar a "grabar" la configuración
        glBindVertexArray(vao_id);

        // 1. Configuramos el Buffer de POSICIONES (Location 0 en el shader)
        glBindBuffer(GL_ARRAY_BUFFER, vbo_ids[0]);
        glBufferData(GL_ARRAY_BUFFER, pos.size() * sizeof(float), pos.data(), GL_STATIC_DRAW);
        // Le decimos a OpenGL: "En el canal 0, lee grupos de 3 floats"
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
        glEnableVertexAttribArray(0);

        // 2. Configuramos el Buffer de TEXTURAS UV (Location 1 en el shader)
        glBindBuffer(GL_ARRAY_BUFFER, vbo_ids[1]);
        glBufferData(GL_ARRAY_BUFFER, uvs.size() * sizeof(float), uvs.data(), GL_STATIC_DRAW);
        // Le decimos: "En el canal 1, lee grupos de 2 floats"
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, 0);
        glEnableVertexAttribArray(1);

        // 3. Configuramos el Buffer de NORMALES (Location 2 en el shader)
        glBindBuffer(GL_ARRAY_BUFFER, vbo_ids[2]);
        glBufferData(GL_ARRAY_BUFFER, norm.size() * sizeof(float), norm.data(), GL_STATIC_DRAW);
        // Le decimos: "En el canal 2, lee grupos de 3 floats"
        glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, 0);
        glEnableVertexAttribArray(2);
    }

    Cube::~Cube() {
        // Borramos buffers y VAO de la GPU al destruir el objeto
        glDeleteVertexArrays(1, &vao_id);
        glDeleteBuffers(3, vbo_ids);
    }

    void Cube::render() {
        // Para dibujar, solo hay que activar el VAO (que ya recuerda la config)
        glBindVertexArray(vao_id);
        // Y mandar dibujar los triángulos
        glDrawArrays(GL_TRIANGLES, 0, vertex_count);
    }
}