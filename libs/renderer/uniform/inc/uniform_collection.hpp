#ifndef UNIFORM_COLLECTION_HPP_INCLUDED
#define UNIFORM_COLLECTION_HPP_INCLUDED

#include <memory>
#include <string_view>
#include <vector>

#include "renderer/uniform/inc/uniform.hpp"

namespace renderer {
namespace uniform {
class UniformCollection : public Uniform {
public:
    void     addMember(std::unique_ptr<Uniform> f_member, int f_position = -1);
    Uniform* getMember(std::string_view f_name);

private:
    std::vector<std::unique_ptr<Uniform>> m_members;
};

}   // namespace uniform
}   // namespace renderer

#endif
