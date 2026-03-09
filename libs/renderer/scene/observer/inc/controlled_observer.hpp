#ifndef CONTROLLED_OBSERVER_HPP_INCLUDED
#define CONTROLLED_OBSERVER_HPP_INCLUDED

#include <glm/glm.hpp>

#include "renderer/scene/observer/inc/observer.hpp"
#include "renderer/window/inc/window.hpp"

namespace renderer {
namespace scene {
class ControlledObserver : public Observer {
public:
    ControlledObserver() = default;

    virtual void update(float f_deltaTime) override;

    void setWindow(window::Window* f_window);

    virtual ~ControlledObserver() = default;

private:
    window::Window* m_window{ nullptr };
    glm::ivec2      m_lastCursorPosition{ -1, -1 };
    bool            m_isDraging{ false };
};
}   // namespace scene
}   // namespace renderer


#endif
