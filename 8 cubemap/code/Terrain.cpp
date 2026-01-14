// Terrain.cpp
// angel.rodriguez@udit.es

#include "Terrain.hpp"
#include <glm.hpp>
#include <half.hpp>
#include <vector>
#include <SOIL2.h>
#include <iostream>
#include <cmath>

using glm::vec3;
using std::vector;
using half_float::half;

namespace udit
{

    Terrain::Terrain(const std::string& heightmap_path, float width, float depth, unsigned x_slices, unsigned z_slices, float max_height)
    {
        // 1. CARGA DEL HEIGHTMAP
        int tex_w = 0, tex_h = 0, tex_ch = 0;
        unsigned char* image = SOIL_load_image(heightmap_path.c_str(), &tex_w, &tex_h, &tex_ch, SOIL_LOAD_RGB);

        if (!image) std::cerr << "ERROR: No se pudo cargar heightmap." << std::endl;

        unsigned n_verts_x = x_slices + 1;
        unsigned n_verts_z = z_slices + 1;
        unsigned total_vertices = n_verts_x * n_verts_z;

        // Buffers temporales
        vector< half > coordinates;
        vector< half > texture_uvs;
        vector< half > normals;         // <--- NUEVO

        coordinates.reserve(total_vertices * 3);
        texture_uvs.reserve(total_vertices * 2);
        normals.reserve(total_vertices * 3);

        // Guardamos las alturas en un vector temporal de floats para poder calcular normales después
        vector<float> temp_heights(total_vertices);

        float x_step = width / float(x_slices);
        float z_step = depth / float(z_slices);

        // --- PASE 1: Generar Geometría y guardar alturas ---
        for (unsigned z = 0; z < n_verts_z; ++z)
        {
            for (unsigned x = 0; x < n_verts_x; ++x)
            {
                float x_pos = -width * 0.5f + x * x_step;
                float z_pos = -depth * 0.5f + z * z_step;
                float y_pos = 0.0f;

                if (image)
                {
                    int img_x = (int)((float)x / x_slices * (tex_w - 1));
                    int img_y = (int)((float)z / z_slices * (tex_h - 1));
                    int pixel_idx = (img_y * tex_w + img_x) * 3; // Asumimos 3 canales por SOIL_LOAD_RGB

                    unsigned char val = image[pixel_idx];
                    y_pos = (float)val / 255.0f * max_height;
                    y_pos -= max_height * 0.15f;
                }

                temp_heights[z * n_verts_x + x] = y_pos;

                coordinates.push_back(half(x_pos));
                coordinates.push_back(half(y_pos));
                coordinates.push_back(half(z_pos));

                float u = (float)x / (float)x_slices;
                float v = (float)z / (float)z_slices;
                texture_uvs.push_back(half(u));
                texture_uvs.push_back(half(v));
            }
        }

        if (image) SOIL_free_image_data(image);

        // --- PASE 2: Calcular Normales ---
        for (unsigned z = 0; z < n_verts_z; ++z)
        {
            for (unsigned x = 0; x < n_verts_x; ++x)
            {
                // Obtenemos las alturas de los vecinos (Left, Right, Down, Up)
                // Si estamos en el borde, usamos la propia altura para no salirnos
                float h_L = (x > 0) ? temp_heights[z * n_verts_x + (x - 1)] : temp_heights[z * n_verts_x + x];
                float h_R = (x < n_verts_x - 1) ? temp_heights[z * n_verts_x + (x + 1)] : temp_heights[z * n_verts_x + x];
                float h_D = (z > 0) ? temp_heights[(z - 1) * n_verts_x + x] : temp_heights[z * n_verts_x + x];
                float h_U = (z < n_verts_z - 1) ? temp_heights[(z + 1) * n_verts_x + x] : temp_heights[z * n_verts_x + x];

                // Calculamos vectores tangentes
                // Tangente en X: (step*2, delta_height_x, 0)
                // Tangente en Z: (0, delta_height_z, step*2)
                glm::vec3 normal(h_L - h_R, 2.0f * x_step, h_D - h_U); // Simplificación del producto cruz
                normal = glm::normalize(normal);

                normals.push_back(half(normal.x));
                normals.push_back(half(normal.y));
                normals.push_back(half(normal.z));
            }
        }

        // --- PASE 3: Índices ---
        vector< GLuint > indices;
        for (unsigned z = 0; z < z_slices; ++z)
        {
            for (unsigned x = 0; x < x_slices; ++x)
            {
                GLuint tl = (z * n_verts_x) + x;
                GLuint tr = (z * n_verts_x) + (x + 1);
                GLuint bl = ((z + 1) * n_verts_x) + x;
                GLuint br = ((z + 1) * n_verts_x) + (x + 1);

                indices.push_back(tl); indices.push_back(bl); indices.push_back(tr);
                indices.push_back(tr); indices.push_back(bl); indices.push_back(br);
            }
        }
        number_of_indices = (GLsizei)indices.size();

        // --- OPENGL CONFIG ---
        glGenVertexArrays(1, &vao_id);
        glGenBuffers(VBO_COUNT, vbo_ids);

        glBindVertexArray(vao_id);

        // 1. Position
        glBindBuffer(GL_ARRAY_BUFFER, vbo_ids[COORDINATES_VBO]);
        glBufferData(GL_ARRAY_BUFFER, coordinates.size() * sizeof(half), coordinates.data(), GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_HALF_FLOAT, GL_FALSE, 0, 0);

        // 2. UVs
        glBindBuffer(GL_ARRAY_BUFFER, vbo_ids[TEXTURE_UVS_VBO]);
        glBufferData(GL_ARRAY_BUFFER, texture_uvs.size() * sizeof(half), texture_uvs.data(), GL_STATIC_DRAW);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_HALF_FLOAT, GL_FALSE, 0, 0);

        // 3. NORMALS (Location 2) <--- IMPORTANTE
        glBindBuffer(GL_ARRAY_BUFFER, vbo_ids[NORMALS_VBO]);
        glBufferData(GL_ARRAY_BUFFER, normals.size() * sizeof(half), normals.data(), GL_STATIC_DRAW);
        glEnableVertexAttribArray(2); // Location 2 en el shader
        glVertexAttribPointer(2, 3, GL_HALF_FLOAT, GL_FALSE, 0, 0);

        // 4. Indices
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo_ids[INDICES_EBO]);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLuint), indices.data(), GL_STATIC_DRAW);
    }

    Terrain::~Terrain()
    {
        glDeleteVertexArrays(1, &vao_id);
        glDeleteBuffers(VBO_COUNT, vbo_ids);
    }

    void Terrain::render()
    {
        glBindVertexArray(vao_id);
        glDrawElements(GL_TRIANGLES, number_of_indices, GL_UNSIGNED_INT, 0);
    }
}