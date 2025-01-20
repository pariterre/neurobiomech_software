#ifndef __NEUROBIO_UTILS_ROLLING_VECTOR_H__
#define __NEUROBIO_UTILS_ROLLING_VECTOR_H__

#include <iostream>
#include <vector>

#include "Utils/CppMacros.h"

namespace NEUROBIO_NAMESPACE::utils {

template <typename T> class RollingVector {

  /// --- Iterator class --- ///
public:
  class Iterator {
  public:
    Iterator(const RollingVector *vec, size_t pos) : m_Vec(vec), m_Pos(pos) {}

    // Dereference the iterator (calls the vector's operator[])
    const T &operator*() const { return (*m_Vec)[m_Pos]; }

    // Pointer access
    const T *operator->() const { return &(*m_Vec)[m_Pos]; }

    // Pre-increment
    Iterator &operator++() {
      m_Pos++;
      return *this;
    }

    // Post-increment
    Iterator operator++(int) {
      Iterator tmp = *this;
      ++(*this);
      return tmp;
    }

    friend Iterator operator+(const Iterator &a, size_t b) {
      return Iterator(a.m_Vec, a.m_Pos + b);
    }

    friend Iterator operator+(size_t a, const Iterator &b) { return b + a; }

    friend Iterator operator-(const Iterator &a, size_t b) {
      return Iterator(a.m_Vec, a.m_Pos - b);
    }

    friend Iterator operator-(size_t a, const Iterator &b) { return b - a; }

    // Equality operator
    friend bool operator==(const Iterator &a, const Iterator &b) {
      return a.m_Pos == b.m_Pos;
    }

    // Inequality operator
    friend bool operator!=(const Iterator &a, const Iterator &b) {
      return !(a == b);
    }

  private:
    const RollingVector *m_Vec;
    size_t m_Pos;
  };

public:
  /// @brief Constructor without a limit (equivalent to std::vector)
  RollingVector()
      : m_MaxSize(-1), m_CurrentIndex(0), m_UnwrapIndex(0), m_IsFull(false) {}

  /// @brief Constructor with a buffer limit
  /// @param size The maximum size of the buffer until it rolls
  RollingVector(size_t size)
      : m_Data(size), m_MaxSize(size), m_CurrentIndex(0), m_UnwrapIndex(0),
        m_IsFull(false) {}

  void setMaxSize(size_t size) {
    m_MaxSize = size;
    clear();
  }

  /// @brief Add a new value to the vector, if the vector is full, the oldest
  /// value is replaced
  /// @param value The value to add
  void push_back(T value) {
    if (m_MaxSize == size_t(-1)) {
      m_Data.push_back(std::move(value));
    } else {
      m_Data[m_CurrentIndex] = value;
    }

    m_CurrentIndex = (m_CurrentIndex + 1) % m_MaxSize;
    m_UnwrapIndex++;

    // Mark as full if we've wrapped around.
    if (m_CurrentIndex == 0) {
      m_IsFull = true;
    }
  }

  // Iterators for range-based for loops.
  RollingVector::Iterator begin() const {
    return RollingVector::Iterator(this, 0);
  }

  RollingVector::Iterator end() const {
    return RollingVector::Iterator(this, m_IsFull ? m_MaxSize : m_CurrentIndex);
  }

  const T &front() const {
    if (m_UnwrapIndex == 0) {
      throw std::out_of_range("Index out of range");
    }
    return *begin();
  }

  const T &back() const {
    if (m_UnwrapIndex == 0) {
      throw std::out_of_range("Index out of range");
    }
    return *(end() - 1);
  }

  /// @brief Clear the vector
  void clear() {
    m_Data.clear();
    if (m_MaxSize != size_t(-1)) {
      m_Data.resize(m_MaxSize);
    }
    m_CurrentIndex = 0;
    m_UnwrapIndex = 0;
    m_IsFull = false;
  }

  /// @brief Get the number of data added to the vector. For unlimited vectors,
  /// this is the same as [std::vector::size].
  /// @return The number of data added to the vector
  size_t size() const { return m_UnwrapIndex; }

  /// @brief Get the requested value. It does not perform any check on the index
  /// @param index The index of the value to get
  /// @return The value at the given index
  const T &operator[](size_t index) const {
    // return m_Data[m_IsFull ? (index + m_CurrentIndex) % m_MaxSize : index];
    //  call the non-const operator[] to avoid code duplication
    return const_cast<RollingVector<T> *>(this)->operator[](index);
  }

  /// @brief Get the requested value. It does not perform any check on the index
  /// @param index The index of the value to get
  /// @return The value at the given index
  T &operator[](size_t index) {
    return m_Data[m_IsFull ? (index + m_CurrentIndex) % m_MaxSize : index];
  }

  /// @brief Get the requested value. It does perform a check on the index
  /// @param index The index of the value to get
  /// @return The value at the given index
  const T &at(size_t index) const {
    if (index >= m_UnwrapIndex) {
      throw std::out_of_range("Index out of range");
    }
    return (*this)[index];
  }

  /// @brief Get the requested value. It does perform a check on the index
  /// @param index The index of the value to get
  /// @return The value at the given index
  T &at(size_t index) {
    if (index >= m_UnwrapIndex) {
      throw std::out_of_range("Index out of range");
    }
    return (*this)[index];
  }

protected:
  /// @brief The data stored in the vector
  DECLARE_PROTECTED_MEMBER_NOGET(std::vector<T>, Data);

  /// @brief The maximum size of the vector
  DECLARE_PROTECTED_MEMBER(size_t, MaxSize);

  /// @brief The index of the last added value
  DECLARE_PROTECTED_MEMBER_NOGET(size_t, CurrentIndex);

  /// @brief The index of the last added value if it was not wrapped
  DECLARE_PROTECTED_MEMBER_NOGET(size_t, UnwrapIndex);

  /// @brief If the vector is full
  DECLARE_PROTECTED_MEMBER(bool, IsFull);
};

} // namespace NEUROBIO_NAMESPACE::utils

#endif // __NEUROBIO_UTILS_ROLLING_VECTOR_H__