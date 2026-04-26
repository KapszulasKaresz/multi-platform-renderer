#ifndef MATERIAL_HPP_INCLUDED
#define MATERIAL_HPP_INCLUDED

#include <memory>
#include <string>
#include <string_view>
#include <vector>

#include "renderer/render_resource/inc/render_resource.hpp"
#include "renderer/uniform/inc/uniform_array.hpp"
#include "renderer/uniform/inc/uniform_collection.hpp"

namespace renderer {
namespace material {
class Material : public RenderResource {
public:
    virtual Material& setShader(std::string f_shaderPath);
    virtual Material& copyMaterial(std::shared_ptr<Material> f_other);
    virtual Material& create() = 0;
    virtual Material& addUniformCollection(
        std::shared_ptr<uniform::UniformCollection> f_uniformCollection
    );

    virtual std::shared_ptr<uniform::UniformCollection> getUniformCollection(
        std::string_view f_name
    );

    virtual Material& addUniformArray(
        std::shared_ptr<uniform::UniformArray> f_uniformArray
    );
    virtual std::shared_ptr<uniform::UniformArray> getUniformArray(
        std::string_view f_name
    );

    bool isOriginal() const;

    void updateUniforms();

protected:
    std::vector<std::shared_ptr<uniform::UniformCollection>> m_uniformCollections;
    std::vector<std::shared_ptr<uniform::UniformArray>>      m_uniformArrays;

    std::string m_shaderLocation{ "" };

    std::shared_ptr<Material> m_original{ nullptr };
};
}   // namespace material
}   // namespace  renderer


#endif
