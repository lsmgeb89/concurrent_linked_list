#include <cstdlib>
#include "tester.h"

int main(int argc, char* argv[]) {
  if (argc != 5) {
    std::cerr << "Wrong Parameter!" << std::endl;
    std::string p(argv[0]);
    std::cerr << p.substr(p.rfind('/') + 1)
              << " <thread_num> <operation_num> <test_times> <max_key>" << std::endl;
    return EXIT_FAILURE;
  }

  std::size_t thread_num(static_cast<std::size_t>(std::stoul(argv[1])));
  std::size_t operation_num(static_cast<std::size_t>(std::stoul(argv[2])));
  std::size_t test_times(static_cast<std::size_t>(std::stoul(argv[3])));
  std::size_t max_key(static_cast<std::size_t>(std::stoul(argv[4])));

  try {
    utils::TestThroughput thru_read("read-dominated",
                                    {std::make_pair(utils::Search, 0.9f),
                                     std::make_pair(utils::Insert, 0.09f),
                                     std::make_pair(utils::Delete, 0.01f)});
    utils::TestThroughput thru_mix("mixed",
                                   {std::make_pair(utils::Search, 0.7f),
                                    std::make_pair(utils::Insert, 0.2f),
                                    std::make_pair(utils::Delete, 0.1f)});
    utils::TestThroughput thru_write("write-dominated",
                                     {std::make_pair(utils::Search, 0.0f),
                                      std::make_pair(utils::Insert, 0.5f),
                                      std::make_pair(utils::Delete, 0.5f)});
    std::vector<utils::TestThroughput> v = {thru_read, thru_mix, thru_write};
    utils::Tester t(thread_num, operation_num, test_times, v, {0, max_key});
    t.Test();
    debug_cout << t.ResultToString();
  } catch (...) {
    std::cerr << "Internal Error. Test Aborted!\n";
  }

  return EXIT_SUCCESS;
}
