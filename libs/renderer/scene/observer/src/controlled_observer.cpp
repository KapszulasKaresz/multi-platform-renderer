#include "renderer/scene/observer/inc/controlled_observer.hpp"

#include <iostream>

#include "renderer/scene/camera/inc/perspective_camera.hpp"
#include "renderer/window/inc/window.hpp"

namespace renderer {
namespace scene {
void ControlledObserver::update(float f_deltaTime)
{
    if (!m_window || !getCamera()) {
        return;
    }

    if (auto l_perspectiveCamera = dynamic_cast<PerspectiveCamera*>(getCamera())) {
        float l_movementSpeed = 5.0f;   // TODO : make this configurable

        // WASD for movement
        auto l_pressedKeys = m_window->getPressedKeys();
        for (auto l_key : l_pressedKeys) {
            if (l_key == window::KEY_W) {
                l_perspectiveCamera->setPosition(
                    l_perspectiveCamera->getPosition()
                    + l_perspectiveCamera->getFront() * f_deltaTime * l_movementSpeed
                );
            }
            if (l_key == window::KEY_S) {
                l_perspectiveCamera->setPosition(
                    l_perspectiveCamera->getPosition()
                    - l_perspectiveCamera->getFront() * f_deltaTime * l_movementSpeed
                );
            }
            if (l_key == window::KEY_A) {
                auto l_right = glm::normalize(
                    glm::cross(
                        l_perspectiveCamera->getFront(), l_perspectiveCamera->getUp()
                    )
                );
                l_perspectiveCamera->setPosition(
                    l_perspectiveCamera->getPosition()
                    - l_right * f_deltaTime * l_movementSpeed
                );
            }
            if (l_key == window::KEY_D) {
                auto l_right = glm::normalize(
                    glm::cross(
                        l_perspectiveCamera->getFront(), l_perspectiveCamera->getUp()
                    )
                );
                l_perspectiveCamera->setPosition(
                    l_perspectiveCamera->getPosition()
                    + l_right * f_deltaTime * l_movementSpeed
                );
            }
            if (l_key == window::KEY_E) {
                l_perspectiveCamera->setPosition(
                    l_perspectiveCamera->getPosition()
                    + l_perspectiveCamera->getUp() * f_deltaTime * l_movementSpeed
                );
            }
            if (l_key == window::KEY_Q) {
                l_perspectiveCamera->setPosition(
                    l_perspectiveCamera->getPosition()
                    - l_perspectiveCamera->getUp() * f_deltaTime * l_movementSpeed
                );
            }
        }

        // Mouse movement for looking around
        float      l_mouseSpeed            = 1.0f;   // TODO : Make this configurable
        bool       l_isDragingCurrently    = false;
        glm::ivec2 l_currentCursorPosition = m_window->getCursorPosition();
        if (m_window->getPressedMouseButtons().size() > 0) {
            for (auto l_mouseButton : m_window->getPressedMouseButtons()) {
                if (l_mouseButton == window::MOUSE_BUTTON_RIGHT) {
                    l_isDragingCurrently = true;
                }
            }
        }

        if (l_isDragingCurrently != m_isDraging) {
            m_window->hideCursor(l_isDragingCurrently);
        }

        if (l_isDragingCurrently) {
            glm::vec2 l_cursorDelta =
                glm::vec2(l_currentCursorPosition - m_lastCursorPosition);

            float l_yaw   = l_cursorDelta.x * l_mouseSpeed * f_deltaTime;
            float l_pitch = -l_cursorDelta.y * l_mouseSpeed * f_deltaTime;

            glm::vec3 l_front = l_perspectiveCamera->getFront();
            glm::vec3 l_up    = l_perspectiveCamera->getUp();
            glm::vec3 l_right = glm::normalize(glm::cross(l_front, l_up));

            if (l_yaw != 0.0f) {
                float cosYaw = cos(l_yaw);
                float sinYaw = sin(l_yaw);
                l_front      = cosYaw * l_front + sinYaw * l_right;
            }

            if (l_pitch != 0.0f) {
                float cosPitch = cos(l_pitch);
                float sinPitch = sin(l_pitch);
                l_front        = cosPitch * l_front + sinPitch * l_up;
            }
            l_perspectiveCamera->setFront(glm::normalize(l_front));
        }

        m_lastCursorPosition = l_currentCursorPosition;
        m_isDraging          = l_isDragingCurrently;
    }
}

void ControlledObserver::setWindow(window::Window* f_window)
{
    m_window = f_window;
}
}   // namespace scene
}   // namespace renderer
