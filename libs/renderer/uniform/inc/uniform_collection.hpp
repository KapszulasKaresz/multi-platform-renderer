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
    Uniform& setType(UniformType f_type) override final;

    void     addMember(std::unique_ptr<Uniform> f_member, int f_position = -1);
    Uniform* getMember(std::string_view f_name);
    virtual UniformSingle* addMember(const std::string& f_name) = 0;

    void addTexture(std::shared_ptr<texture::Texture> f_textrue, int f_position = -1);
    texture::Texture* getTexture(std::string_view f_name);

    virtual void update() = 0;

protected:
    std::vector<std::unique_ptr<Uniform>>          m_members;
    std::vector<std::shared_ptr<texture::Texture>> m_textures;
};

}   // namespace uniform
}   // namespace renderer

#endif
