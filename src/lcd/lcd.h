#include <cstdint>
#include <string>

namespace lcd {
    bool initialize(void);
    bool is_initialized(void);
    bool print(std::int16_t line, std::string text);
    bool clear_line(std::int16_t line);
    bool clear();
}