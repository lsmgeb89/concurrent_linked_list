#ifndef CONCURRENT_LINKED_LIST_TESTER_H_
#define CONCURRENT_LINKED_LIST_TESTER_H_

#include <algorithm>
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <iostream>
#include <random>
#include <sstream>
#include <thread>
#include <vector>
#include "coarse_grained_linked_list.h"
#include "fine_grained_linked_list.h"
#include "lock_free_linked_list.h"
#include "log_util.h"

namespace utils {

enum OperationType {
  Search = 0,
  Insert = 1,
  Delete = 2
};

struct TestOperation {
  OperationType type_;
  int parameter_;
};

typedef std::pair<OperationType, float> OpThroughput;
typedef std::chrono::duration<double, std::nano> TestResult;

class TestThroughput {
 public:
  TestThroughput(const std::string name,
                 const std::array<OpThroughput, 3>& profile)
    : name_(name),
      profile_(profile) {}

  std::string ToString(void) {
    std::stringstream out_stream;
    out_stream << name_ << ": ";

    for (auto p : profile_) {
      out_stream << p.second;
      if (Search == p.first) {
        out_stream << " Search ";
      } else if (Insert == p.first) {
        out_stream << " Insert ";
      } else {
        out_stream << " Delete ";
      }
    }

    return out_stream.str();
  }

 public:
  std::string name_;
  std::array<OpThroughput, 3> profile_;
};

#define CALL_FUNC_IF_MATCH(FUNC_NAME)                               \
if (operation.type_ == FUNC_NAME) {                                 \
  debug_clog << "[thread " << id << "] " << linked_list_.name_      \
             << "::"#FUNC_NAME"(" << operation.parameter_ << ")\n"; \
  ret = linked_list_.FUNC_NAME(operation.parameter_);               \
  debug_clog << "[thread " << id << "] " << linked_list_.name_      \
             << "::"#FUNC_NAME"(" << operation.parameter_ << ") = " \
             << std::boolalpha << ret << "\n";                      \
}

template <typename ListType> class UnitTester {
 public:
  UnitTester(void) {}
  ~UnitTester(void) {}

  static std::string GetName(void) { return ListType::name_; }

  void ThreadFunc(const std::size_t& id, const std::vector<TestOperation>& operation_list) {
    bool ret;
    for (auto operation : operation_list) {
      CALL_FUNC_IF_MATCH(Search)
      CALL_FUNC_IF_MATCH(Insert)
      CALL_FUNC_IF_MATCH(Delete)
    }
  }

  TestResult UnitTest(const std::vector<std::vector<TestOperation>>& operation_list_group) {
    auto begin = std::chrono::steady_clock::now();

    for (std::size_t i(0); i < operation_list_group.size(); i++) {
      this->thread_pool.emplace_back([this, operation_list_group, i] { this->ThreadFunc(i, operation_list_group.at(i)); });
    }

    for (auto& thread : thread_pool) {
      thread.join();
    }

    auto end = std::chrono::steady_clock::now();

#ifndef NDEBUG
    std::clog << "[" << linked_list_.name_ << "] Thread = " << operation_list_group.size() << ", Total Time = "
              << std::chrono::duration_cast<std::chrono::nanoseconds>(end - begin).count()
              << " (ns)" << std::endl;
#endif

    thread_pool.clear();
    return (end - begin);
  }

 private:
  ListType linked_list_;
  std::vector<std::thread> thread_pool;
};

class Tester {
 public:
  Tester(const std::size_t& max_thread_num,
         const std::size_t& operation_num,
         const std::size_t& repeat_times,
         const std::vector<TestThroughput>& throughput_list)
    : max_thread_num_(max_thread_num),
      operation_num_(operation_num),
      repeat_times_(repeat_times),
      throughput_list_(throughput_list),
      dist_(1, 100) {
    random_engine_.seed(std::random_device()());
    // fix the sizes
    test_results_.resize(throughput_list.size());
    for (auto& test_result : test_results_) {
      std::get<0>(test_result).resize(max_thread_num_);
      std::get<1>(test_result).resize(max_thread_num_);
      std::get<2>(test_result).resize(max_thread_num_);
    }
  }

  void GenerateOperations(const TestThroughput& throughput,
                          const std::size_t& thread_num,
                          const std::size_t& operation_num,
                          std::vector<std::vector<TestOperation>>& operation_list_group) {
    auto operation_per_thread(operation_num / thread_num);
    std::vector<TestOperation> operations;
    operations.resize(operation_num);
    operation_list_group.resize(thread_num);
    for (auto& op_list : operation_list_group) {
      op_list.resize(operation_per_thread);
    }

    // generate operations
    auto op_iter(operations.begin());
    for (auto p : throughput.profile_) {
      auto limit(std::floor(p.second * operation_num));
      for (auto i(0); i < limit; ++i) {
        *op_iter++ = {p.first, static_cast<int>(dist_(random_engine_))};
      }
    }
    for (; op_iter != operations.end(); ++op_iter) {
      *op_iter = {throughput.profile_.at(dist_(random_engine_) % throughput.profile_.size()).first,
                  static_cast<int>(dist_(random_engine_))};
    }

    // randomly sort all operations
    std::random_shuffle(operations.begin(), operations.end());

    // distribute operations to each thread
    auto j(operations.begin());
    for (auto i(operation_list_group.begin()); i != operation_list_group.end(); ++i, j += operation_per_thread) {
      std::copy(j, j + operation_per_thread, (*i).begin());
    }

    // distribute remaining to threads randomly
    for (; j != operations.end(); ++j) {
      operation_list_group.at(dist_(random_engine_) % thread_num).push_back(*j);
    }
  }

  void Test(void) {
    for (std::size_t i = 0; i < throughput_list_.size(); i++) {
      for (std::size_t t_num = 1; t_num <= max_thread_num_; t_num++) {
        // For each thread number, we test multiple times
        for (std::size_t r = 0; r < repeat_times_; r++) {
          std::vector<std::vector<TestOperation>> operation_list_group;
          GenerateOperations(throughput_list_.at(i), t_num, operation_num_, operation_list_group);

          utils::UnitTester<utils::LockedLinkedList> coarse_grained_tester;
          utils::UnitTester<utils::LazyLinkedList> fine_grained_tester;
          utils::UnitTester<utils::LockFreeLinkedList> lock_free_tester;
          std::get<0>(test_results_.at(i)).at(t_num - 1) += coarse_grained_tester.UnitTest(operation_list_group);
          std::get<1>(test_results_.at(i)).at(t_num - 1) += fine_grained_tester.UnitTest(operation_list_group);
          std::get<2>(test_results_.at(i)).at(t_num - 1) += lock_free_tester.UnitTest(operation_list_group);
        }

        // average
        std::get<0>(test_results_.at(i)).at(t_num - 1) /= repeat_times_;
        std::get<1>(test_results_.at(i)).at(t_num - 1) /= repeat_times_;
        std::get<2>(test_results_.at(i)).at(t_num - 1) /= repeat_times_;
      }
    }
  }

  std::string ResultToString(void) {
    std::stringstream out_stream;

    for (std::size_t i = 0; i < throughput_list_.size(); i++) {
      // parameter
      out_stream << throughput_list_.at(i).ToString()
                 << "Thread Number: 1 ~ " << max_thread_num_
                 << ", Operation Number: " << operation_num_
                 << ", test times: " << repeat_times_
                 << ", Time Unit: Nanosecond"
                 << std::endl;

      // header
      out_stream << "ThreadNumber, "
                 << utils::UnitTester<utils::LockedLinkedList>::GetName() << ", "
                 << utils::UnitTester<utils::LazyLinkedList>::GetName() << ", "
                 << utils::UnitTester<utils::LockFreeLinkedList>::GetName() << std::endl;

      // line
      for (std::size_t j = 0; j < max_thread_num_; j++) {
        out_stream << j + 1 << ", "
                   << std::get<0>(test_results_.at(i)).at(j).count() << ", "
                   << std::get<1>(test_results_.at(i)).at(j).count() << ", "
                   << std::get<2>(test_results_.at(i)).at(j).count() << std::endl;
      }

      out_stream << std::endl;
    }

    return out_stream.str();
  }

 private:
  std::size_t max_thread_num_;
  std::size_t operation_num_;
  std::size_t repeat_times_;
  std::vector<TestThroughput> throughput_list_;

  std::mt19937 random_engine_;
  std::uniform_int_distribution<std::mt19937::result_type> dist_;

  std::vector<std::tuple<std::vector<TestResult>, std::vector<TestResult>, std::vector<TestResult>>> test_results_;
};

} // namespace utils

#endif // CONCURRENT_LINKED_LIST_TESTER_H_
