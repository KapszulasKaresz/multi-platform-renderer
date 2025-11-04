#ifndef UTILS_HPP_INCLUDED
#define UTILS_HPP_INCLUDED

#include <string>
#include <vector>

namespace renderer {
namespace utils {
std::vector<char> readFile(const std::string& f_filename);

inline size_t alignUp(size_t f_offset, size_t f_alignment)
{
    return (f_offset + f_alignment - 1) & ~(f_alignment - 1);
}

}   // namespace utils
}   // namespace renderer


#endif
