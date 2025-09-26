#ifndef MATERIAL_HPP_INCLUDED
#define MATERIAL_HPP_INCLUDED

#include <memory>
#include <string>
#include <string_view>
#include <vector>

#include "renderer/render_resource/inc/render_resource.hpp"
#include "renderer/uniform/inc/uniform_collection.hpp"

namespace renderer {
namespace material {
class Material : public RenderResource {
public:
    virtual Material& setShader(std::string f_shaderPath);
    virtual Material& create() = 0;
    virtual Material& addUniformCollection(
        std::shared_ptr<uniform::UniformCollection> f_uniformCollection
    );

    virtual std::shared_ptr<uniform::UniformCollection> getUniformCollection(
        std::string_view f_name
    );

protected:
    std::vector<std::shared_ptr<uniform::UniformCollection>> m_uniformCollections;

    std::string m_shaderLocation{ "" };
};
}   // namespace material
}   // namespace  renderer


#endif
