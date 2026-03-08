#ifndef NODE_3D_HPP_INCLUDED
#define NODE_3D_HPP_INCLUDED

#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>

#include "renderer/scene/inc/node.hpp"

namespace renderer {
namespace scene {
class Node3D : public Node {
public:
    Node3D();
    Node3D& create();

    virtual void applyVisitor(NodeVisitor* f_visitor) override;

    glm::mat4 getTransform() const;
    Node3D&   setTransform(const glm::mat4& f_transform);
    Node3D&   setTransform(
          const glm::vec3& f_translation,
          const glm::quat& f_rotation,
          const glm::vec3& f_scale
      );
    Node3D& setRotation(const glm::quat& f_rotation);
    Node3D& setTranslation(const glm::vec3& f_translation);
    Node3D& setScale(const glm::vec3& f_scale);

    virtual ~Node3D() = default;

protected:
    void updateTransform();

    glm::mat4 m_transform{ 1.0f };
    glm::mat4 m_inverseTransform{ 1.0f };

    glm::vec3 m_translation{ 0.0f, 0.0f, 0.0f };
    glm::quat m_rotation{ 1.0f, 0.0f, 0.0f, 0.0f };
    glm::vec3 m_scale{ 1.0f, 1.0f, 1.0f };
};
}   // namespace scene
}   // namespace renderer

#endif
