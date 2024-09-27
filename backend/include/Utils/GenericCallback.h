#include <functional>
#include <map>

template <typename T> class GenericCallback {
public:
  void listen(std::function<void(const T &)> callback);
  void clear(std::function<void(const T &)> callback);

  void notifyListeners(const T &data);

protected:
  std::map<std::function<void(const T &)>, std::function<void(const T &)>>
      m_Callbacks;
};