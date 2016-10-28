#ifndef CONCURRENT_LINKED_LIST_FINE_GRAINED_LINKED_LIST_H_
#define CONCURRENT_LINKED_LIST_FINE_GRAINED_LINKED_LIST_H_

#include <limits>
#include <mutex>
#include <sstream>
#include "list_node.h"

namespace utils {

class LazyLinkedList {
 public:
  LazyLinkedList(void)
      : head_(std::numeric_limits<int>::min(), &tail_, false),
        tail_(std::numeric_limits<int>::max(), nullptr, false) {}

  ~LazyLinkedList() {
    LockedListNode* curr(head_.next_);
    LockedListNode* tmp(nullptr);
    while (curr != &tail_) {
      tmp = curr;
      curr = curr->next_;
      debug_clog << "~LazyLinkedList free " << tmp->val_ << std::endl;
      delete tmp;
    }
  }

  bool Search(const int& value) {
    LockedListNode* curr(head_.next_);
    while (curr != &tail_) {
      if (curr->val_ == value && !curr->marked_) {
        return true;
      }
      curr = curr->next_;
    }
    return false;
  }

  bool Insert(const int& value) {
    while(true) {
      // find a window
      LockedListWindow scan_window(LocateWindow(value));

      // lock the window
      WindowGuard<LockedListWindow> guard(scan_window);

      // validate the window
      if (Validate(scan_window)) {
        LockedListNode *pred(scan_window.first);
        LockedListNode *curr(scan_window.second);
        if (curr->val_ == value) {
          return false;
        } else {
          pred->next_ = new LockedListNode(value, curr, false);
          return true;
        }
      }
      // if validation failed, just retry
    }
  }

  bool Delete(const int& value) {
    while(true) {
      // find a window
      LockedListWindow scan_window(LocateWindow(value));

      // lock the window
      WindowGuard<LockedListWindow> guard(scan_window);

      // validate the window
      if (Validate(scan_window)) {
        LockedListNode *pred(scan_window.first);
        LockedListNode *curr(scan_window.second);
        if (curr->val_ != value) {
          return false;
        } else {
          curr->marked_ = true;
          pred->next_ = curr->next_;
          // TODO: find a way to delete it
          // delete curr;
          return true;
        }
      }
      // if validation failed, just retry
    }
  }

  std::string ToString(void) {
    std::stringstream ss;
    LockedListNode* curr(head_.next_);
    while (curr != &tail_) {
      ss << curr->val_;
      if (curr->next_ != &tail_) {
        ss << " ";
      }
      curr = curr->next_;
    }
    return ss.str();
  }

 private:
  LockedListNode head_;
  LockedListNode tail_;

  LockedListWindow LocateWindow(const int& key) {
    LockedListNode* pred(&head_);
    LockedListNode* curr(head_.next_);
    while (curr->val_ < key) {
      pred = curr;
      curr = curr->next_;
    }
    return std::make_pair(pred, curr);
  }

  bool Validate(const LockedListWindow& window) const {
    LockedListNode* pred(window.first);
    LockedListNode* curr(window.second);
    return (!pred->marked_ && !curr->marked_ && pred->next_ == curr);
  }

 public:
  static constexpr auto name_ = "LazyLinkedList";
};

} // namespace utils

#endif // CONCURRENT_LINKED_LIST_FINE_GRAINED_LINKED_LIST_H_
