#ifndef TEXTURE_HPP_INCLUDED
#define TEXTURE_HPP_INCLUDED

#include <memory>
#include <string>

#include "renderer/image/inc/image.hpp"
#include "renderer/render_resource/inc/render_resource.hpp"

namespace renderer {
namespace texture {
enum Filter {
    FILTER_NEAREST,
    FILTER_LINEAR,
    FILTER_CUBIC,
    FILTER_MAX
};

enum MipMapMode {
    MIPMAP_MODE_NEAREST,
    MIPMAP_MODE_LINEAR,
    MIPMAP_MODE_MAX
};

enum AddressMode {
    ADDRESS_MODE_REPEAT,
    ADDRESS_MODE_MIRRORED_REPEAT,
    ADDRESS_MODE_CLAMP_TO_EDGE,
    ADDRESS_MODE_CLAMP_TO_BORDER,
    ADDRESS_MODE_MIRRORED_CLAMP_TO_EDGE,
    ADDRESS_MODE_MAX
};

class Texture : public RenderResource {
public:
    virtual Texture& setImage(std::shared_ptr<image::Image> f_image);
    virtual Texture& create() = 0;

    std::string getName() const;
    Texture&    setName(const std::string& f_name);

    Texture& setMinFilter(Filter f_filter);
    Texture& setMagFilter(Filter f_filter);
    Texture& setMipMapMode(MipMapMode f_mode);

    Texture& setAddressModeU(AddressMode f_addressMode);
    Texture& setAddressModeV(AddressMode f_addressMode);
    Texture& setAddressModeW(AddressMode f_addressMode);

protected:
    std::shared_ptr<image::Image> m_image{ nullptr };
    std::string                   m_name{ "" };

    Filter     m_minFilter{ FILTER_LINEAR };
    Filter     m_magFilter{ FILTER_LINEAR };
    MipMapMode m_mipMapMode{ MIPMAP_MODE_LINEAR };

    AddressMode m_addressModeU{ ADDRESS_MODE_REPEAT };
    AddressMode m_addressModeV{ ADDRESS_MODE_REPEAT };
    AddressMode m_addressModeW{ ADDRESS_MODE_REPEAT };
};
}   // namespace texture
}   // namespace renderer

#endif
