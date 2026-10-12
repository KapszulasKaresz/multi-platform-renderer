#ifndef MATERIAL_HPP_INCLUDED
#define MATERIAL_HPP_INCLUDED

#include <memory>
#include <string>
#include <string_view>
#include <vector>

#include "renderer/render_resource/inc/render_resource.hpp"
#include "renderer/uniform/inc/uniform_array.hpp"
#include "renderer/uniform/inc/uniform_acceleration_structure.hpp"
#include "renderer/uniform/inc/uniform_collection.hpp"
#include "renderer/uniform/inc/uniform_storage_buffer.hpp"

namespace renderer {
namespace texture {
class Texture;
}

namespace material {
enum MaterialType {
    MATERIAL_TYPE_RENDER,
    MATERIAL_TYPE_COMPUTE,
    MATERIAL_TYPE_RAY_TRACING,
    MATERIAL_TYPE_MAX
};

class Material : public RenderResource {
public:
    Material&    setMaterialType(MaterialType f_materialType);
    MaterialType getMaterialType() const;

    virtual Material& setShader(std::string f_shaderPath);
    virtual Material& setRayTracingAlbedoTextures(
        std::vector<std::shared_ptr<texture::Texture>> f_textures
    );
    virtual Material& setRayTracingNormalTextures(
        std::vector<std::shared_ptr<texture::Texture>> f_textures
    );
    virtual Material& setRayTracingMetallicRoughnessTextures(
        std::vector<std::shared_ptr<texture::Texture>> f_textures
    );
    virtual Material& setRayTracingGeometryBuffer(
        std::shared_ptr<uniform::UniformStorageBuffer> f_buffer
    );
    virtual Material& copyMaterial(std::shared_ptr<Material> f_other);
    virtual Material& create() = 0;

    virtual Material& addUniformCollection(
        std::shared_ptr<uniform::UniformCollection> f_uniformCollection
    );
    virtual std::shared_ptr<uniform::UniformCollection> getUniformCollection(
        std::string_view f_name
    );

    virtual Material& addUniformStorageBuffer(
        std::shared_ptr<uniform::UniformStorageBuffer> f_uniformStorageBuffer
    );
    virtual std::shared_ptr<uniform::UniformStorageBuffer> getUniformStorageBuffer(
        std::string_view f_name
    );

    virtual Material& addUniformAccelerationStructure(
        std::shared_ptr<uniform::UniformAccelerationStructure> f_uniform
    );
    virtual std::shared_ptr<uniform::UniformAccelerationStructure>
        getUniformAccelerationStructure(std::string_view f_name);

    virtual Material& addUniformArray(
        std::shared_ptr<uniform::UniformArray> f_uniformArray
    );
    virtual std::shared_ptr<uniform::UniformArray> getUniformArray(
        std::string_view f_name
    );

    bool isOriginal() const;

    void updateUniforms();

protected:
    std::vector<std::shared_ptr<uniform::UniformCollection>>    m_uniformCollections;
    std::vector<std::shared_ptr<uniform::UniformArray>>         m_uniformArrays;
    std::vector<std::shared_ptr<uniform::UniformStorageBuffer>> m_uniformStorageBuffers;
    std::vector<std::shared_ptr<uniform::UniformAccelerationStructure>>
        m_uniformAccelerationStructures;

    std::string m_shaderLocation{ "" };
    std::vector<std::shared_ptr<texture::Texture>> m_rayTracingAlbedoTextures;
    std::vector<std::shared_ptr<texture::Texture>> m_rayTracingNormalTextures;
    std::vector<std::shared_ptr<texture::Texture>>
        m_rayTracingMetallicRoughnessTextures;
    std::shared_ptr<uniform::UniformStorageBuffer> m_rayTracingGeometryBuffer;

    std::shared_ptr<Material> m_original{ nullptr };

    MaterialType m_materialType{ MaterialType::MATERIAL_TYPE_RENDER };
};
}   // namespace material
}   // namespace  renderer


#endif
