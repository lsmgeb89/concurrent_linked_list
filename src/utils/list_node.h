#ifndef CONCURRENT_LINKED_LIST_LIST_NODE_H_
#define CONCURRENT_LINKED_LIST_LIST_NODE_H_

#include <atomic>

namespace utils {

class ListNode {
 public:
  ListNode(const int& val, ListNode* const next)
    : val_(val),
      next_(next) {}

  ~ListNode(void) {
    next_ = nullptr;
  }

  int val_;
  ListNode* next_;
};

class AtomicListNode {
 public:
  AtomicListNode(const int& val, AtomicListNode* const next)
    : val_(val),
      next_(next) {}

  ~AtomicListNode(void) {
    next_ = nullptr;
  }

  int val_;
  std::atomic<AtomicListNode*> next_;
};

class LockedListNode {
 public:
  LockedListNode(const int& val,
                 LockedListNode* const next,
                 const bool& marked)
    : val_(val),
      next_(next),
      marked_(marked) {}

  ~LockedListNode(void) {
    next_ = nullptr;
  }

  void Lock(void) { mutex_.lock(); }

  void Unlock(void) { mutex_.unlock(); }

  int val_;
  LockedListNode* next_;
  bool marked_;
  std::mutex mutex_;
};

typedef std::pair<AtomicListNode*, AtomicListNode*> ListWindow;
typedef std::pair<LockedListNode*, LockedListNode*> LockedListWindow;

template <typename WindowType> class WindowGuard {
 public:
  WindowGuard(WindowType& list_window)
    : window_(list_window) {
    window_.first->Lock();
    window_.second->Lock();
  }

  ~WindowGuard(void) {
    window_.second->Unlock();
    window_.first->Unlock();
  }
 private:
  WindowType window_;
};

// only work for x86-64 platform
template <typename PointerType>
inline PointerType ExtractPointer(PointerType pointer) {
  return reinterpret_cast<PointerType>((reinterpret_cast<int64_t>(pointer) << 1) >> 1);
}

template <typename PointerType>
inline PointerType MarkPointer(PointerType pointer) {
  return reinterpret_cast<PointerType>(reinterpret_cast<int64_t>(pointer) | (static_cast<int64_t>(1) << (sizeof(PointerType) * 8 - 1)));
}

template <typename PointerType>
inline bool IsMarked(PointerType pointer) {
  return static_cast<bool>(reinterpret_cast<int64_t>(pointer) >> (sizeof(PointerType) * 8 - 1));
}

} // namespace utils

#endif // CONCURRENT_LINKED_LIST_LIST_NODE_H_
