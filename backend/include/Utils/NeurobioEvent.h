#ifndef __NEUROBIO_UTILS_NEUROBIO_EVENT_H__
#define __NEUROBIO_UTILS_NEUROBIO_EVENT_H__

#include <functional>
#include <map>
#include <shared_mutex>

#include "Utils/CppMacros.h"

namespace NEUROBIO_NAMESPACE::utils {

template <typename T> class NeurobioEvent {
public:
  ///
  /// @brief Listen to the callback. This method returns an id that can be used
  /// to refer to the callback later on (e.g. to remove it)
  /// @param callback The callback to call when the event is triggered
  /// @return The id of the callback
  size_t listen(std::function<void(const T &)> callback) {
    std::unique_lock lock(m_Mutex);

    static size_t id = 0;
    size_t newId = id;
    m_Callbacks[newId] = callback;
    id++;
    return newId;
  }

  ///
  /// @brief Remove the callback associated with the given id. This must be
  /// called otherwise the callback will be kept in memory
  /// @param id The id of the callback to remove
  void clear(size_t id) {
    std::unique_lock lock(m_Mutex);
    m_Callbacks.erase(id);
  }

  ///
  /// @brief Notify all the listeners that the event has been triggered
  /// @param data The data to pass to the listeners
  void notifyListeners(const T &data) {
    std::shared_lock lock(m_Mutex);
    for (const auto &[_, callback] : m_Callbacks) {
      callback(data);
    }
  }

protected:
  std::map<size_t, std::function<void(const T &)>> m_Callbacks;

  DECLARE_PROTECTED_MEMBER_NOGET(std::shared_mutex, Mutex)
};

} // namespace NEUROBIO_NAMESPACE::utils

#endif // __NEUROBIO_UTILS_NEUROBIO_EVENT_H__