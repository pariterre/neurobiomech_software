#include "Utils/GenericCallback.h"

template <typename T>
void GenericCallback<T>::listen(std::function<void(const T &)> callback) {
  m_Callbacks[callback] = callback;
}

template <typename T>
void GenericCallback<T>::clear(std::function<void(const T &)> callback) {
  m_Callbacks.erase(callback);
}

template <typename T> void GenericCallback<T>::notifyListeners(const T &data) {
  for (const auto &[_, callback] : m_Callbacks) {
    callback(data);
  }
}