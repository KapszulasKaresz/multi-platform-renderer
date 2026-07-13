#include <optional>
#include <ranges>
#include <string>
#include <string_view>
#include <vector>

class InputParser {
public:
    explicit InputParser(int f_argc, const char** f_argv)
    {
        m_tokens.reserve(static_cast<size_t>(f_argc > 0 ? f_argc - 1 : 0));
        for (int i = 1; i < f_argc; ++i) {
            m_tokens.emplace_back(f_argv[i]);
        }
    }

    [[nodiscard]]
    std::optional<std::string_view> getCmdOption(std::string_view f_option) const
    {
        auto l_it = std::ranges::find(m_tokens, f_option);
        if (l_it != m_tokens.end() && std::next(l_it) != m_tokens.end()) {
            return std::string_view{ *std::next(l_it) };
        }
        return std::nullopt;
    }

    [[nodiscard]]
    bool cmdOptionExists(std::string_view f_option) const
    {
        return std::ranges::find(m_tokens, f_option) != m_tokens.end();
    }

private:
    std::vector<std::string> m_tokens;
};
