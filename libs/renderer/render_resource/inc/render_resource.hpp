#ifndef RENDER_RESOURCE_HPP_INCLUDED
#define RENDER_RESOURCE_HPP_INCLUDED

#include <memory>
#include <vector>

namespace renderer {
class RenderResource {
public:
    virtual bool isValid();

protected:
    bool m_valid{ false };
};

using RenderResourceStore = std::vector<std::unique_ptr<RenderResource>>;
}   // namespace renderer

#endif
