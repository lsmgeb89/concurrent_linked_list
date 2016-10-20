#ifndef CONCURRENT_LINKED_LIST_LOCK_FREE_LINKED_LIST_H_
#define CONCURRENT_LINKED_LIST_LOCK_FREE_LINKED_LIST_H_

#include <atomic>
#include <limits>
#include <sstream>
#include "list_node.h"

namespace utils {

class LockFreeLinkedList {
 public:
  LockFreeLinkedList(void)
    : head_(std::numeric_limits<int>::min(), &tail_),
      tail_(std::numeric_limits<int>::max(), nullptr) {}

  ~LockFreeLinkedList(void) {
    AtomicListNode* curr(head_.next_.load());
    AtomicListNode* tmp(nullptr);
    while (curr != &tail_) {
      tmp = curr;
      curr = ExtractPointer(curr->next_.load());
      std::cout << "free " << tmp->val_ << std::endl;
      delete tmp;
    }
  }

  bool Search(const int& value) {
    ListWindow window = LocateWindow(value);
    AtomicListNode* curr(window.second);
    return (curr->val_ == value && !IsMarked(curr->next_.load()));
  }

  bool Insert(const int& value) {
    while(true) {
      // find a window
      ListWindow window(LocateWindow(value));
      AtomicListNode* pred(window.first);
      AtomicListNode* curr(window.second);

      // already exists
      if (curr->val_ == value) {
        return false;
      } else {
        // directly set curr means being unmarked
        AtomicListNode* new_node(new AtomicListNode(value, curr));
        // if pred->next == curr then pred->next = new_node
        bool res(std::atomic_compare_exchange_weak(&(pred->next_), &curr, new_node));
        if (res) { return true; }
        // if failed, just retry
      }
    }
  }

  bool Delete(const int& value) {
    while(true) {
      // find a window
      ListWindow window = LocateWindow(value);
      AtomicListNode* pred(window.first);
      AtomicListNode* curr(window.second);

      // no such a value
      if (curr->val_ != value) {
        return false;
      } else {
        AtomicListNode* unmarked_succ(ExtractPointer(curr->next_.load()));
        AtomicListNode* marked_succ(MarkPointer(unmarked_succ));
        // validate and mark: res := CAS(curr->next, <0, succ>, <1, succ>)
        bool res(std::atomic_compare_exchange_weak(&(curr->next_), &unmarked_succ, marked_succ));
        if (res) {
          AtomicListNode* unmarked_curr(ExtractPointer(curr));
          // change pointer: CAS(pred->next, <0, curr>, <0, succ>)
          std::atomic_compare_exchange_weak(&(pred->next_), &unmarked_curr, unmarked_succ);
          // TODO: find a way to delete it
          delete unmarked_curr;
          return true;
        }
        // if validation failed, just retry
      }
    }
  }

  std::string ToString(void) {
    std::stringstream ss;
    AtomicListNode* curr(head_.next_);
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
  AtomicListNode head_;
  AtomicListNode tail_;

  ListWindow LocateWindow(const int& key) {
  retry:
    while (true) {
      AtomicListNode* unmarked_pred(&head_);
      AtomicListNode* unmarked_curr(ExtractPointer(head_.next_.load()));

      while (true) {
        AtomicListNode* unmarked_succ(ExtractPointer(unmarked_curr->next_.load()));
        bool marked_flag(IsMarked(unmarked_curr->next_.load()));
        // clear all marked node while moving forward
        while (marked_flag) {
          bool res(std::atomic_compare_exchange_weak(&(unmarked_pred->next_), &unmarked_curr, unmarked_succ));
          if (!res) {
            goto retry;
          } else {
            // TODO: find a way to delete it
            delete unmarked_curr;
            // move forward
            unmarked_curr = unmarked_succ;
            unmarked_succ = ExtractPointer(unmarked_curr->next_.load());
            marked_flag = IsMarked(unmarked_curr->next_.load());
          }
        }

        // find a window
        if (unmarked_curr->val_ >= key) {
          return std::make_pair(unmarked_pred, unmarked_curr);
        }

        // move forward
        unmarked_pred = unmarked_curr;
        unmarked_curr = unmarked_succ;
      }
    }
  }
};

} // namespace utils

#endif // CONCURRENT_LINKED_LIST_LOCK_FREE_LINKED_LIST_H_
