#ifndef NODE_HEADER
#define NODE_HEADER

#include <glm.hpp>
#include <vector>
#include <memory>
#include <functional>

namespace udit {
    class Node {
    public:
        // Matriz de transformación LOCAL del nodo. 
        // Almacena posición, rotación y escala relativas a su padre, no al mundo global.
        glm::mat4 transform;

        // Lista de punteros inteligentes a los nodos hijos.
        // Si mueves este nodo, todos los hijos se moverán con él automáticamente.
        std::vector<std::shared_ptr<Node>> children;

        // Función "callback" para dibujar. 
        // En lugar de obligar a heredar de Node para dibujar cosas distintas (Cubo, Terreno...),
        // le asignamos una función lambda que contiene el código específico de dibujo de OpenGL.
        std::function<void(const glm::mat4&)> render_callback;

        // Constructor: Inicializa la matriz identidad (sin movimiento).
        Node();

        // Añade un nodo hijo a la jerarquía de este nodo.
        void add_child(std::shared_ptr<Node> child);

        // Función RECURSIVA clave: 
        // 1. Calcula la posición real en el mundo.
        // 2. Dibuja el objeto (si tiene callback).
        // 3. Llama a esta misma función en todos sus hijos.
        void update_and_render(const glm::mat4& parent_transform);
    };
}
#endif