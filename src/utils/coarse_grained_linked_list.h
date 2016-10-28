#ifndef CONCURRENT_LINKED_LIST_COARSE_GRAINED_LINKED_LIST_H_
#define CONCURRENT_LINKED_LIST_COARSE_GRAINED_LINKED_LIST_H_

#include <limits>
#include <mutex>
#include <sstream>
#include "list_node.h"
#include "log_util.h"

namespace utils {

class LockedLinkedList {
 public:
  LockedLinkedList(void)
    : head_(std::numeric_limits<int>::min(), &tail_),
      tail_(std::numeric_limits<int>::max(), nullptr) {}

  ~LockedLinkedList() {
    ListNode* curr(head_.next_);
    ListNode* tmp(nullptr);
    while (curr != &tail_) {
      tmp = curr;
      curr = curr->next_;
      debug_clog << "~LockedLinkedList free " << tmp->val_ << std::endl;
      delete tmp;
    }
  }

  bool Search(const int& value) {
    ListNode* curr(head_.next_);
    while (curr && curr != &tail_) {
      if (curr->val_ == value) {
        return true;
      }
      curr = curr->next_;
    }
    return false;
  }

  bool Insert(const int& value) {
    std::lock_guard<std::mutex> guard(mutex_);
    ListNode* pred(&head_);
    ListNode* curr(head_.next_);
    while (curr) {
      if (curr->val_ > value) {
        pred->next_ = new ListNode(value, curr);
        break;
      }
      pred = pred->next_;
      curr = curr->next_;
    }
    // always return true for testing
    return true;
  }

  bool Delete(const int& value) {
    std::lock_guard<std::mutex> guard(mutex_);
    ListNode* pred(&head_);
    ListNode* curr(head_.next_);
    while (curr != &tail_) {
      if (curr->val_ == value) {
        pred->next_ = curr->next_;
        delete curr;
        return true;
      }
      pred = pred->next_;
      curr = curr->next_;
    }
    return false;
  }

  std::string ToString(void) {
    std::stringstream ss;
    ListNode* curr(head_.next_);
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
  std::mutex mutex_;
  ListNode head_;
  ListNode tail_;

 public:
  static constexpr auto name_ = "LockedLinkedList";
};

} // namespace utils

#endif // CONCURRENT_LINKED_LIST_COARSE_GRAINED_LINKED_LIST_H_
