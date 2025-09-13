#include "renderer/scene/inc/test_scene.hpp"

#include "renderer/material/inc/material.hpp"

namespace renderer {
namespace scene {
TestScene& TestScene::setMaterial(std::shared_ptr<material::Material> f_material)
{
    m_material = f_material;
    return *this;
}

TestScene& TestScene::create()
{
    m_valid = true;
    return *this;
}
}   // namespace scene
}   // namespace renderer
