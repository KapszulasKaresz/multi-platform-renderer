#ifndef CAMERA_HPP_INCLUDED
#define CAMERA_HPP_INCLUDED

#include <glm/glm.hpp>

namespace renderer {
namespace scene {

class Camera {
public:
    Camera() = default;

    virtual glm::mat4 V() const = 0;
    virtual glm::mat4 P() const = 0;

    virtual ~Camera() = default;

protected:
};
}   // namespace scene
}   // namespace renderer

#endif
