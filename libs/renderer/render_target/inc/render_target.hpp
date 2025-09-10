#ifndef RENDER_TARGET_HPP_INCLUDED
#define RENDER_TARGET_HPP_INCLUDED

namespace renderer {
namespace render_target {

class RenderTarget {
public:
    virtual bool isValid() const;

protected:
    bool m_valid{ false };
};

}   // namespace render_target
}   // namespace renderer

#endif
