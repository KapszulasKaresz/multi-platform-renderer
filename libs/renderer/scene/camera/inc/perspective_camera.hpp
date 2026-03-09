#ifndef PERSPECTIVE_CAMERA_HPP_INCLUDED
#define PERSPECTIVE_CAMERA_HPP_INCLUDED

#include "renderer/scene/camera/inc/camera.hpp"

namespace renderer {
namespace scene {
class PerspectiveCamera : public Camera {
public:
    PerspectiveCamera();

    virtual glm::mat4 V() const override;
    virtual glm::mat4 P() const override;

    PerspectiveCamera& setPosition(const glm::vec3& f_position);
    PerspectiveCamera& setFront(const glm::vec3& f_front);
    PerspectiveCamera& setUp(const glm::vec3& f_up);
    PerspectiveCamera& setFovY(float f_fovY);
    PerspectiveCamera& setAspectRatio(float f_aspectRatio);
    PerspectiveCamera& setNearPlane(float f_nearPlane);
    PerspectiveCamera& setFarPlane(float f_farPlane);

    glm::vec3 getPosition() const;
    glm::vec3 getFront() const;
    glm::vec3 getUp() const;
    float     getFovY() const;
    float     getAspectRatio() const;
    float     getNearPlane() const;
    float     getFarPlane() const;

    ~PerspectiveCamera() = default;

protected:
    glm::vec3 m_position{ 0.0f, 0.0f, 0.0f };
    glm::vec3 m_front{ 1.0f, 0.0f, 0.0f };
    glm::vec3 m_up{ 0.0f, 1.0f, 0.0f };

    float m_fovY{ glm::radians(45.0f) };
    float m_aspectRatio{ 16.0f / 9.0f };
    float m_nearPlane{ 0.1f };
    float m_farPlane{ 100.0f };
};
}   // namespace scene
}   // namespace renderer

#endif
