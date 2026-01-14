#ifndef NODE_HEADER
#define NODE_HEADER

#include <glm.hpp>
#include <vector>
#include <memory>
#include <functional>

namespace udit {
    class Node {
    public:
        glm::mat4 transform;
        std::vector<std::shared_ptr<Node>> children;
        std::function<void(const glm::mat4&)> render_callback;

        Node();
        void add_child(std::shared_ptr<Node> child);
        void update_and_render(const glm::mat4& parent_transform);
    };
}
#endif