#ifndef UNIFORM_STORAGE_BUFFER_HPP_INCLUDED
#define UNIFORM_STORAGE_BUFFER_HPP_INCLUDED

#include <array>
#include <span>
#include <vector>

#include "renderer/uniform/inc/uniform.hpp"

namespace renderer {
namespace uniform {

class UniformStorageBuffer : public Uniform {
public:
    UniformStorageBuffer();

    size_t getSize() const override final;

    virtual void setValueRaw(std::span<const std::byte> f_dataView) = 0;

    template <typename T>
    void setValue(const T& f_data)
    {
        auto l_view = std::span<const std::byte>(
            reinterpret_cast<const std::byte*>(&f_data), sizeof(T)
        );
        setValueRaw(l_view);
    }

    template <typename T>
    void setValue(const std::vector<T>& f_dataList)
    {
        auto l_view = std::span<const std::byte>(
            reinterpret_cast<const std::byte*>(f_dataList.data()),
            sizeof(T) * f_dataList.size()
        );
        setValueRaw(l_view);
    }

    template <typename T, size_t N>
    void setValue(const std::array<T, N>& f_dataArray)
    {
        auto l_view = std::span<const std::byte>(
            reinterpret_cast<const std::byte*>(f_dataArray.data()),
            sizeof(T) * f_dataArray.size()
        );
        setValueRaw(l_view);
    }

    template <typename T, size_t N>
    void setValue(const T (&f_dataArray)[N])
    {
        auto l_view = std::span<const std::byte>(
            reinterpret_cast<const std::byte*>(f_dataArray), sizeof(T) * N
        );
        setValueRaw(l_view);
    }

    virtual void getValueRaw(std::span<std::byte> f_outBuffer) = 0;

    template <typename T>
    void getValue(T& f_outData)
    {
        auto l_view =
            std::span<std::byte>(reinterpret_cast<std::byte*>(&f_outData), sizeof(T));
        getValueRaw(l_view);
    }

    template <typename T>
    void getValue(std::vector<T>& f_outVector)
    {
        if (f_outVector.empty()) {
            return;
        }
        auto l_view = std::span<std::byte>(
            reinterpret_cast<std::byte*>(f_outVector.data()),
            f_outVector.size() * sizeof(T)
        );
        getValueRaw(l_view);
    }

    template <typename T, size_t N>
    void getValue(std::array<T, N>& f_outArray)
    {
        auto l_view = std::span<std::byte>(
            reinterpret_cast<std::byte*>(f_outArray.data()), f_outArray.size() * sizeof(T)
        );
        getValueRaw(l_view);
    }

    template <typename T, size_t N>
    void getValue(T (&f_outArray)[N])
    {
        auto l_view =
            std::span<std::byte>(reinterpret_cast<std::byte*>(f_outArray), sizeof(T) * N);
        getValueRaw(l_view);
    }

protected:
    size_t m_size{ 0 };
};

}   // namespace uniform
}   // namespace renderer

#endif   // UNIFORM_STORAGE_BUFFER_HPP_INCLUDED
