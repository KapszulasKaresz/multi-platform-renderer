#ifndef OBSERVER_HPP_INCLUDED
#define OBSERVER_HPP_INCLUDED

namespace renderer {
namespace scene {
class Camera;

class Observer {
public:
    Observer() = default;

    virtual void update(float f_deltaTime) = 0;

    void    setCamera(Camera* f_camera);
    Camera* getCamera() const;

    virtual ~Observer() = default;

private:
    Camera* m_camera{ nullptr };
};
}   // namespace scene
}   // namespace renderer


#endif
