#ifndef TEST_SCENE_HPP_INCLUDED
#define TEST_SCENE_HPP_INCLUDED

#include "renderer/render_resource/inc/render_resource.hpp"

namespace renderer {
namespace material {
class Material;
}   // namespace material

namespace scene {
class TestScene : public RenderResource {
public:
    TestScene& setMaterial(std::shared_ptr<material::Material> f_material);
    TestScene& create();

private:
    std::shared_ptr<material::Material> m_material{ nullptr };
};
}   // namespace scene
}   // namespace renderer

#endif
