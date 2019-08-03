#include <util.h>

std::string ConvertToString(float number) {
    std::ostringstream buff;
    buff.precision(3);
    buff << std::fixed;
    buff << number;
    return buff.str();
}
std::string ConvertToString(int number) {
    std::ostringstream buff;
    buff << number;
    return buff.str();
}
std::string ConvertToString(long number) {
    std::ostringstream buff;
    buff << number;
    return buff.str();
}
