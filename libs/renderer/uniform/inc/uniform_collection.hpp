#ifndef UNIFORM_COLLECTION_HPP_INCLUDED
#define UNIFORM_COLLECTION_HPP_INCLUDED

#include <memory>
#include <string>
#include <string_view>
#include <vector>

#include "renderer/texture/inc/texture.hpp"
#include "renderer/uniform/inc/uniform.hpp"
#include "renderer/uniform/inc/uniform_single.hpp"

namespace renderer {
namespace uniform {
class UniformCollection : public Uniform {
public:
    UniformCollection();

    UniformCollection& setType(UniformType f_type) override final;
    UniformCollection& setUnique(bool f_unique = true);

    bool isUniqueCollection() const;

    void     addMember(std::shared_ptr<Uniform> f_member, int f_position = -1);
    Uniform* getMember(std::string_view f_name);
    virtual UniformSingle* addMember(const std::string& f_name) = 0;

    virtual void
        addTexture(std::shared_ptr<texture::Texture> f_textrue, int f_position = -1);
    texture::Texture* getTexture(std::string_view f_name);
    texture::Texture* getTexture(int f_position);

    virtual std::shared_ptr<UniformCollection> deepCopy() const = 0;

    int          getTextureCount() const;
    virtual void update() = 0;

protected:
    bool m_isUnique{ true };

    std::vector<std::shared_ptr<Uniform>>          m_members;
    std::vector<std::shared_ptr<texture::Texture>> m_textures;
};

}   // namespace uniform
}   // namespace renderer

#endif
