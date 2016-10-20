#include <cassert>
#include <iostream>
#include "coarse_grained_linked_list.h"
#include "fine_grained_linked_list.h"
#include "lock_free_linked_list.h"

template <typename ListType>
void simple_test(ListType& test_list) {
  assert(!test_list.Delete(2));
  assert(!test_list.Search(3));
  test_list.Insert(4);
  assert(!test_list.Delete(2));
  assert(!test_list.Search(3));
  test_list.Insert(10);
  assert(test_list.Delete(4));
  assert(test_list.Search(10));
  assert(test_list.Delete(10));
  test_list.Insert(19);
  test_list.Insert(20);
  test_list.Insert(17);
  std::cout << test_list.ToString() << std::endl;
  assert(test_list.Delete(20));
  std::cout << test_list.ToString() << std::endl;
  assert(test_list.Delete(17));
  std::cout << test_list.ToString() << std::endl;
};

int main(int argc, char* argv[]) {
  utils::LockedLinkedList list_1;
  utils::LazyLinkedList list_2;
  utils::LockFreeLinkedList list_3;

  simple_test(list_1);
  simple_test(list_2);
  simple_test(list_3);

  return 0;
}
