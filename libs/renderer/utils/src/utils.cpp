#include "renderer/utils/inc/utils.hpp"

#include <fstream>
#include <iostream>

namespace renderer {
namespace utils {
std::vector<char> readFile(const std::string& f_filename)
{
    std::ifstream l_file(f_filename, std::ios::ate | std::ios::binary);
    if (!l_file.is_open()) {
        throw std::runtime_error(
            "readFile(const std::string& f_filename) failed to open file!: " + f_filename
        );
    }
    std::vector<char> l_buffer(l_file.tellg());
    l_file.seekg(0, std::ios::beg);
    l_file.read(l_buffer.data(), static_cast<std::streamsize>(l_buffer.size()));
    l_file.close();
    return l_buffer;
}
}   // namespace utils
}   // namespace renderer
