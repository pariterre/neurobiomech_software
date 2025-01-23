#ifndef __NEUROBIO_UTILS_CPP_MACROS_H__
#define __NEUROBIO_UTILS_CPP_MACROS_H__

/// -------------------- ///
/// GENERIC DECLARATIONS ///
/// -------------------- ///

// Define a "DECLARE_PROTECTED_MEMBER_WITH_SETTER" macro that will declare a
// protected member variable and a public getter and setter for it. The macro
// does not capitalize the first letter of the member variable name, so it
// should be used with PascalCase names so the getter is in camelCase.
#define DECLARE_PROTECTED_MEMBER_WITH_SETTER(type, name)                       \
public:                                                                        \
  type get##name() const { return m_##name; }                                  \
  void set##name(type value) { m_##name = value; }                             \
                                                                               \
protected:                                                                     \
  type m_##name;

// Define a "DECLARE_PROTECTED_MEMBER" macro that will declare a protected
// member variable and a public getter for it. The macro does not capitalize the
// first letter of the member variable name, so it should be used with
// PascalCase names so the getter is in camelCase.
#define DECLARE_PROTECTED_MEMBER(type, name)                                   \
public:                                                                        \
  const type &get##name() const { return m_##name; }                           \
                                                                               \
protected:                                                                     \
  type m_##name;

// Define a "DECLARE_PROTECTED_MEMBER" macro that will declare a protected
// member variable without accessors. The macro does not capitalize the first
// letter of the member variable name, so it should be used with PascalCase
// names.
#define DECLARE_PROTECTED_MEMBER_NOGET(type, name)                             \
protected:                                                                     \
  type m_##name;

// Define a "DECLARE_PRIVATE_MEMBER" macro that will declare a private
// member variable without accessors. The macro does not capitalize the first
// letter of the member variable name, so it should be used with PascalCase
// names.
#define DECLARE_PRIVATE_MEMBER_NOGET(type, name)                               \
private:                                                                       \
  type m_##name;

/// ---------------------- ///
/// DEVICE COMMANDS MACROS ///
/// ---------------------- ///
#define DECLARE_DEVICE_COMMAND(commandName, value)                             \
public:                                                                        \
  static constexpr int commandName = value;                                    \
  static constexpr const char *commandName##_AS_STRING = #commandName;

#endif // __NEUROBIO_UTILS_CPP_MACROS_H__