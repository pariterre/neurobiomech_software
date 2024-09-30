#ifndef __STIMWALKER_UTILS_STIMWALKER_EVENT_H__
#define __STIMWALKER_UTILS_STIMWALKER_EVENT_H__

#include <functional>
#include <map>

template <typename T> class StimwalkerEvent {
public:
  ///
  /// @brief Listen to the callback. This method returns an id that can be used
  /// to refer to the callback later on (e.g. to remove it)
  /// @param callback The callback to call when the event is triggered
  /// @return The id of the callback
  size_t listen(std::function<void(const T &)> callback) {
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
  void clear(size_t id) { m_Callbacks.erase(size_t); }

  ///
  /// @brief Notify all the listeners that the event has been triggered
  /// @param data The data to pass to the listeners
  void notifyListeners(const T &data) {
    for (const auto &[_, callback] : m_Callbacks) {
      callback(data);
    }
  }

protected:
  std::map<size_t, std::function<void(const T &)>> m_Callbacks;
};

#endif // __STIMWALKER_UTILS_STIMWALKER_EVENT_H__