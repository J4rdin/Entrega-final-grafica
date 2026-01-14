#include "Node.hpp"

namespace udit {

    Node::Node() : transform(1.0f) {}

    void Node::add_child(std::shared_ptr<Node> child) {
        children.push_back(child);
    }

    void Node::update_and_render(const glm::mat4& parent_transform) {
        glm::mat4 world_transform = parent_transform * transform;

        if (render_callback) {
            render_callback(world_transform);
        }

        for (auto& child : children) {
            child->update_and_render(world_transform);
        }
    }
}