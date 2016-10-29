### Compile and run a batch session on TACC
### Firstly, Please modify your parameters in tacc_batch.sh
./build_run_on_tacc.sh

### Compile and run on csgrads1.utdallas.edu
./build_run_on_csgrads1.sh

### Compile and Run manually
### parameter of concurrent_linked_list <thread_num> <operation_num> <test_times> <max_key>
### <thread_num>: Indicate the max threads will be tested. The program will test from 1 ~ <thread_num> threads.
### <operation_num>: number operations of the linked list to be tested
### <test_times>: repeating times of each test
### <max_key>: Indicate the max number of key space. So the key space will be 0 ~ <max_key>
cd src && make && ./concurrent_linked_list 32 1000 16 49
