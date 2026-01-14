#include "Node.hpp"

namespace udit {

    // Constructor: Inicializa la transformación como una matriz identidad (1.0f).
    // Esto significa que por defecto el nodo está en el (0,0,0) sin rotación ni escala.
    Node::Node() : transform(1.0f) {}

    // Añade un hijo al vector 'children'.
    void Node::add_child(std::shared_ptr<Node> child) {
        children.push_back(child);
    }

    // Función principal del Grafo de Escena.
    // Recibe la transformación acumulada del padre (parent_transform).
    void Node::update_and_render(const glm::mat4& parent_transform) {

        // --- PASO 1: Calcular la Matriz de Mundo ---
        // Multiplicamos la matriz del padre por la local de este nodo.
        // El orden es vital: Padre * Hijo. Esto acumula los movimientos.
        // 'world_transform' representa la posición, rotación y escala final en el mundo 3D.
        glm::mat4 world_transform = parent_transform * transform;

        // --- PASO 2: Dibujar ---
        // Si este nodo tiene asignada una función de dibujo (no es un nodo invisible/agrupador),
        // ejecutamos esa función pasándole su matriz de mundo calculada.
        if (render_callback) {
            render_callback(world_transform);
        }

        // --- PASO 3: Propagar a los Hijos ---
        // Recorremos todos los hijos y los llamamos recursivamente,
        // pero ahora 'world_transform' (mi posición) será su 'parent_transform'.
        for (auto& child : children) {
            child->update_and_render(world_transform);
        }
    }
}