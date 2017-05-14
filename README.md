# Concurrent Linked List

## Summary
  * Implemented a concurrent linked list that supports `search`, `insert` and `delete` operations by using the following three approaches:
    1. coarse-grained locking with search operation oblivious to locks
    2. fine-grained locking using lazy synchronization
    3. lock-free synchronization
  * Implemented a common test facility that could generate operations with different distribution and test linked lists by different degree of concurrency (number of threads concurrently operating on a linked list) according to the requirements of testing throughput
  * Compared the averaged performance of the three implementations as a function of number of threads (varied from one to the number of logical cores in the machine) under different distribution of operations

## Project Information
  * Course: Introduction to Multicore Programming (CS 6301)
  * Professor: [Neeraj Mittal][mittal]
  * Semester: Fall 2016
  * Programming Language: C++
  * Build Tool: [CMake][cmake]

[mittal]: http://cs.utdallas.edu/people/faculty/mittal-neeraj
[cmake]: https://cmake.org
