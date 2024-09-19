#ifndef __STIMWALKER_UTILS_CPP_MACROS_H__
#define __STIMWALKER_UTILS_CPP_MACROS_H__

// Define a "DECLARE_PROTECTED_MEMBER" macro that will declare a protected member variable and a public getter for it.
// The macro does not capitalize the first letter of the member variable name, so it should be used with PascalCase names
// so the getter is in camelCase.
#define DECLARE_PROTECTED_MEMBER(type, name) \
public:                                      \
    type get##name() const                   \
    {                                        \
        return m_##name;                     \
    }                                        \
                                             \
protected:                                   \
    type m_##name;

#endif // __STIMWALKER_UTILS_CPP_MACROS_H__