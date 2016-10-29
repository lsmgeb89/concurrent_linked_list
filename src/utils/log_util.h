#ifndef CONCURRENT_LINKED_LIST_LOG_UTIL_H_
#define CONCURRENT_LINKED_LIST_LOG_UTIL_H_

#include <algorithm>
#include <iomanip>
#include <mutex>

static std::mutex mutex_log_;

#ifndef NDEBUG
#define debug_clog std::clog << lock_with(mutex_log_)
#define debug_cout std::clog << lock_with(mutex_log_)
#else
class NullBuffer : public std::streambuf {
public:
  int overflow(int c) { return c; }
};

class NullStream : public std::ostream {
  public:
    NullStream() : std::ostream(&m_sb) {}
  private:
    NullBuffer m_sb;
};
static NullStream null_stream;

#define debug_clog null_stream
#define debug_cout std::cout << lock_with(mutex_log_)
#endif

inline std::ostream& operator<<(std::ostream& out,
                                const std::lock_guard<std::mutex> &) {
  return out;
}

template <typename T> inline std::lock_guard<T> lock_with(T &mutex) {
  mutex.lock();
  return { mutex, std::adopt_lock };
}

#endif // CONCURRENT_LINKED_LIST_LOG_UTIL_H_
