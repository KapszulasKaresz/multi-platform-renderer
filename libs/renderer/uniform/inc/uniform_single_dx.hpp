#ifndef UNIFORM_DX_HPP_INCLUDED
#define UNIFORM_DX_HPP_INCLUDED

#include "renderer/uniform/inc/uniform_single.hpp"

namespace renderer {
namespace uniform {
class UniformSingleDX : public UniformSingle {
public:
    size_t getAlignment() const override final;
};

}   // namespace uniform
}   // namespace renderer
#endif
