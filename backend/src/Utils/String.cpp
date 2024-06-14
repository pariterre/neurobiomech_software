#define STIMWALKER_API_EXPORTS
#include "Utils/String.h"

#include <sstream>
#include <map>
#include <algorithm>
#include <vector>

#include "Utils/Exceptions.h"

using namespace STIMWALKER_NAMESPACE::utils;

String::String()
    : std::string("")
{
}

String::String(
    const char *text)
    : std::string(text)
{
}

String::String(
    const String &text)
    : std::string(text)
{
}

String::String(
    const std::basic_string<char> &text)
    : std::string(text)
{
}

String &String::operator=(
    const String &other)
{
    if (this == &other)
    { // check for self-assigment
        return *this;
    }

    this->std::string::operator=(other);
    return *this;
}

String String::operator+(
    const char *text)
{
    String tp = *this;
    tp.append(text);
    return tp;
}

String String::operator+(
    double val)
{
    return *this + to_string(val);
}

String String::operator+(
    size_t val)
{
    return *this + std::to_string(val);
}

String String::operator+(
    int val)
{
    return *this + std::to_string(val);
}

String String::operator()(
    size_t idx) const
{
    if (idx >= this->length())
    {
        throw OutOfBoundsException("Index for string out of range");
    }

    char out[2];
    out[0] = (*this)[idx];
    out[1] = '\0';
    return out;
}

String String::operator()(
    size_t startIdx,
    size_t lastIdx) const
{
    if (startIdx >= this->length() || lastIdx >= this->length())
    {
        throw OutOfBoundsException("Index for string out of range");
    }
    if (lastIdx <= startIdx)
    {
        throw OutOfBoundsException("Second argument should be higher than first!");
    }

    char *out = static_cast<char *>(malloc(lastIdx - startIdx + 2 * sizeof(char)));
    for (size_t k = 0; k < lastIdx - startIdx + 1; ++k)
    {
        out[k] = (*this)[startIdx + k];
    }
    out[lastIdx - startIdx + 1] = '\0';
    String Out(out);
    free(out);
    return Out;
}

String::~String()
{
}

String String::tolower(
    const String &str)
{
    String new_str = str;
    std::transform(new_str.begin(), new_str.end(), new_str.begin(), ::tolower);
    return new_str;
}

String String::tolower() const
{
    return tolower(*this);
}

String String::toupper(
    const String &str)
{
    String new_str = str;
    std::transform(new_str.begin(), new_str.end(), new_str.begin(), ::toupper);
    return new_str;
}

String String::toupper() const
{
    return toupper(*this);
}

String String::to_string(
    double val)
{
    std::ostringstream out;
    out.precision(20);
    out << std::fixed << val;
    return removeTrailing(out.str(), "0");
}

String String::to_string(
    float val)
{
    std::ostringstream out;
    out.precision(20);
    out << std::fixed << val;
    return removeTrailing(out.str(), "0");
}

String String::removeTrailing(
    const String &origin,
    const String &trailTag)
{
    if (trailTag.length() != 1)
    {
        throw WrongDimensionsException("Tag should be of length 1");
    }
    String out(origin);

    while (out.length() > 0 && out.back() == trailTag[0])
    {
        out.pop_back();
    }
    return out;
}

std::ostream &operator<<(std::ostream &os, const String &a)
{
    os << a.c_str();
    return os;
}
