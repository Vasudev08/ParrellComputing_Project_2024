# CSCE 435 Group project

## 0. Group number: 18

## 1. Group members:

1. Anna Hartman
2. Tate Moskal
3. Nicole Hernandez
4. Vasudev Agarwal

Our team will communicate primarily through Text message for real-time discussions and updates. We will also use GitHub to track code contributions and Trello for project management, ensuring transparency and accountability for task assignments.

## 2. Parallel Sorting Algorithms 

### 2a. Brief project description
In this project, we will be comparing the efficiencies of different sorting algorithms running in parallel. The algorithms will be implemented and tested on parallel architectures, such as multi-core processors and possibly distributed systems. Each team member will focus on one of the following algorithms:

- Bitonic Sort: Anna Hartman
- Sample Sort: Vasudev Agarwal
- Merge Sort: Nicole Hernandez
- Radix Sort: Tate Moskal

The comparison will assess execution time, we will evaluate the efficiency on different input sizes (small, medium, large) to measure scalability, and analyzing how these algorithms leverage parallelism to improve sorting performance.

### 2b. Pseudocode for each parallel algorithm

#### 2b.1 Bitonic Sort
In Bitonic sort, a bitonic sequence is built (a sequence that first increases and then decreases) and sorted by merging. Within multiple threads, chunks of the given sequence are sorted into bitonic order. The merging process is carried out in parallel, merging each piece into a large bitonic sequence. Threads should be synchronized. The entire algorithm is carried out recursively in order to build the final bitonic sequence. 

1. Divide the given array to be sorted into parallel chunks with corresponding threads
2. For each thread ands its piece (in parallel): sort the piece into bitonic order by recursively splitting the piece into two halves and sorting the first half into ascending order and the second into descending order, and then merging the two haves into a bitonic sequence.
3. Synchronize threads by ensuring each thread has completed its sorting before continuing on.
4. Merge all of the pieces bitonically (in parallel), for each chunk and thread: compare and swap such that if the current chunk is in the lower half, merge it in ascending order, and if it is in the upper half, merge it in descending order. Recursively merge the two halves of the sequence to ensure they are fully sorted in either ascending or descending order. This should be carried out log(array_size) times, as each chunk doubles in size after being recursively merged. 
5. Output the result.

Actual Code Algorithm Description:
1. Initializes the MPI environment with MPI_Init, retrieving the rank of each process and the total number of processes using MPI_Comm_rank and MPI_Comm_size. The root process (rank 0) parses command-line arguments to determine the size of the array (as a power of 2) and the number of processes to be used.
2. Root process (rank 0) generates a random array of integers of size 2^exponent. This size is calculated from the exponent passed as an argument. The root process seeds the random number generator with the current time to ensure different data for each run.
3. Root process then divides the array into equal-sized chunks and distributes these chunks to all processes, including itself, using MPI_Scatter. Each process receives a local sub-array (of size total array size / number of processes) for sorting, ensuring that the sorting workload is spread evenly across all processes.
4. Once each process receives its portion of the array, it applies the Bitonic Sort algorithm locally.
   the Bitonic Sort works as follows:
a. Recursively divides the sub-array into smaller parts.
b. Sorts the smaller parts in ascending and descending order alternately.
c. Merges these parts using the Bitonic Merge step, which compares and swaps elements in such a way that the sub-array becomes sorted.
5. After sorting their local sub-arrays, each process sends its sorted sub-array back to the root process using MPI_Gather. The root process collects all the sorted sub-arrays into the original array.
6. Once all sub-arrays are gathered, the root process performs a final Bitonic Sort on the entire array to merge the sorted sub-arrays into a fully sorted array. This final step is necessary because the sub-arrays are only partially sorted, and a global sort ensures that the entire array is in the correct order.
7. The root process then finally checks if the final array is correctly sorted by comparing each element with the next. If any element is out of order, it reports an error; otherwise, it confirms that the array is sorted.
8. Finally, the program shuts down the MPI environment with MPI_Finalize, cleaning up all resources used by MPI.

#### 2b.2 Sample Sort

In sample sort a large dataset is divided into smaller paritions,
after which each partition is sorted independently, and then these
sorted paritions are merged to obtain the final sorted result.

1. Splitting the given dataset into equally divided smaller
   segments, where each segement is given to a processor
2. Run basic sorting algorithm on each of the segments,
   where each processor is handling its piece independently.
3. From these sorted segments, each processor selects
   few samples. then, MPI communication is used to collect samples from all processors
4. Sort the selected samples, which will help us establish a
   global order among the samples.
5. From the sorted samples, we pick few speical elements. which act
   as a pivot. which are shared with all processors. MPI_Bcast is used to broadcast the pivots to all processors.
6. Each processor takes its ordered segment and divides it into
   subsegments based on the choicen pivot.
7. Now, processors share their ordered segments globally with the
   corresponding processor based on the segment number. using MPI_Alltoall
8. Finally, each processor merges and sorts the recieved elements.

#### 2b.3 Merge Sort
Parallel Merge Sort uses the divide and conquer technique, recursively dividing the dataset into smaller parts, sorting them, and merging the results where in parallel ver. it is distributed across multiple processors. Parallel Merge Sort will be implemented using MPI. Each processor will perform its own sorting idenpendently, merging sorted data using MPI communication.

1. Start MPI for Communication between processors.
2. Identify if the process is the master (rank 0) or worker (rank > 0)
3. The master splits the dataset into smaller parts
4. The master sends each part to a worker process
5. Each worker process sorts its part of the dataset independently
6. Workers send their sorted parts back to the master
7. The master merges all sorted parts into one sorted array
8. Close MPI after sorting is complete

#### 2b.4 Radix Sort
Radix Sort is an algorithm that sorts by processing through individual digits, sorting along the way. The process can be sped up by allowing each processor to handle a portion of the total array. By sorting a subarray and keeping note of the order of subarray chunks being sorted in each processor, they can be placed accordingly back into the main array. While this example sorts via binary, the process can account for numbers of any base as long as the # of arrays corresponds correctly.

   Pseudocode:
   1. Initialize MPI for multiprocessor communication
   2. Convert array digits into binary (helps with initial implementation).
   3. Find the maximum element and its # of digits.
   5. Begin iterating through digits starting at the least significant digit up to the maximum digit significance.
   7. Split the array into subarrays depending on the # of processors used and send to workers,
      keeping track of the order in which each subarray gets sent where.
   9. Each worker will sort its subarray into 2 arrays, the first with digits that are 0, the second with digits that are 1.
   10. Worker returns arrays and master combines the 0 array in order of worker process, then repeats for the 1 array.
   11. Repeat with the next digit until all digits places have been parsed.
   12. End MPI once complete.

   Coded Algorithm (Differs from pseudocode as this version more optimally parallelizes Radix Sort):
   1. Initializes MPI environment with MPI_Init(), MPI_Comm_size(), and MPI_Comm_rank().
   2. Master process at rank 0 creates array based on command line arguments.
   3. Total array size is broadcasted to all processes with MPI_Bcast().
   4. Total array is split into subarrays based on # of processers and distributed amongst the processors with MPI_Scatter().
   5. Each process finds the maximum value of its subarray in base 2 as well as the amount of digits it has.
   6. Each process sorts its subarray by digit group in countSort(). A histogram counts the occurances of each digit group in the subarray. Using the histogram and summations, it sorts the local subarray and redistributes the info to the rest of the processes.
   7. Arrays are gathered together, exchanging information and parts of subarrays to globally sort the total array.
   8. Using MPI_Gatherv() All subarrays are collected in the master.
   9. The total array is checked if it is sorted and prints the result.
   10. Memory is deallocated and MPI_Finalize() ends the MPI environment.
   
### 2c. Evaluation plan - what and how will you measure and compare

- Input sizes, Input types (Alter input array to be: random, sorted, reverse sorted,...)
- Strong scaling (same problem size, increase number of processors/nodes)
- Weak scaling (increase problem size, increase number of processors)
- Parallelization strategies (master/worker vs SPMD, calculating speedup and runtime differences)
- Communication strategies (collectives vs point-to-point, measure runtime differences between code for each communication strategy)


### 3a. Caliper 

#### Merge Sort Calltree
```
0.504 main
├─ 0.000 MPI_Init
├─ 0.003 MPI_Bcast
├─ 0.000 comm
│  ├─ 0.000 MPI_Scatter
│  └─ 0.000 comm_large
│     └─ 0.000 MPI_Gather
├─ 0.000 comp
│  └─ 0.000 comp_large
├─ 0.000 MPI_Finalize
├─ 0.000 MPI_Initialized
├─ 0.000 MPI_Finalized
└─ 0.000 MPI_Comm_dup
```
#### Sample Sort Calltree
```
0.271 main
├─ 0.000 data_init_runtime
├─ 0.267 MPI_Init
│  └─ 0.000 MPI_Init
├─ 0.003 comm
│  ├─ 0.001 comm_scatter
│  │  └─ 0.000 MPI_Scatter
│  ├─ 0.000 comm_gather_samples
│  │  └─ 0.000 MPI_Gather
│  ├─ 0.001 comm_bcast_pivots
│  │  └─ 0.001 MPI_Bcast
│  ├─ 0.001 comm_all_to_all
│  │  └─ 0.001 MPI_Alltoall
│  ├─ 0.000 comm_alltoallv
│  │  └─ 0.000 MPI_Alltoallv
│  └─ 0.000 final_gather
│     └─ 0.000 MPI_Gatherv
├─ 0.000 comp
│  ├─ 0.000 local_sort
│  ├─ 0.000 pivot_sort
│  ├─ 0.000 pivot_partition
│  └─ 0.000 merge_elements
├─ 0.000 MPI_Barrier
├─ 0.000 MPI_Gather
├─ 0.000 correctness_check
├─ 0.000 MPI_Finalize
├─ 0.000 MPI_Initialized
├─ 0.000 MPI_Finalized
└─ 0.001 MPI_Comm_dup

```
#### Radix Sort Calltree
```
2.722 main
├─ 0.000 MPI_Init
├─ 0.051 data_init_runtime
├─ 0.852 comm
│  ├─ 0.825 comm_large
│  │  ├─ 0.087 MPI_Bcast
│  │  ├─ 0.055 MPI_Allgather
│  │  └─ 0.680 MPI_Gatherv
│  └─ 0.024 comm_small
│     ├─ 0.013 MPI_Scatter
│     ├─ 0.001 MPI_Gather
│     ├─ 0.006 MPI_Bcast
│     └─ 0.003 MPI_Gatherv
├─ 0.045 comp
│  ├─ 0.000 comp_small
│  └─ 0.044 comp_large
├─ 0.087 MPI_Barrier
├─ 0.049 correctness_check
├─ 0.000 MPI_Finalize
├─ 0.000 MPI_Initialized
├─ 0.000 MPI_Finalized
└─ 0.004 MPI_Comm_dup
```

#### Bitonic Sort Calltree

```
4.972 main
├─ 0.000 MPI_Init
└─ 2.803 comp
   └─ 2.803 comp_large
├─ 2.169 comm
│  └─ 2.169 comm_large
│     ├─ 2.163 MPI_Scatter 
│     └─ 0.007 MPI_Gather
├─ 0.000 correctness_check
├─ 0.000 MPI_Init
├─ 0.000 MPI_Finalize
├─ 0.000 MPI_Finalized
├─ 0.000 MPI_Initialized
└─ 24.052 MPI_Comm_dup
```





### 3b. Collect Metadata

#### Merge Sort Metadata
```
           cali.caliper.version  mpi.world.size  \
profile                                           
3133824840               2.11.0               2   

                                                 spot.metrics  \
profile                                                         
3133824840  min#inclusive#sum#time.duration,max#inclusive#...   

           spot.timeseries.metrics  spot.format.version  \
profile                                                   
3133824840                                            2   

                                                 spot.options  spot.channels  \
profile                                                                        
3133824840  time.variance,profile.mpi,node.order,region.co...  regionprofile   

           cali.channel spot:node.order   spot:output spot:profile.mpi  \
profile                                                                  
3133824840         spot            true  p2-a128.cali             true   

           spot:region.count spot:time.exclusive spot:time.variance algorithm  \
profile                                                                         
3133824840              true                true               true     merge   

           programming_model  group_num  input_size  num_procs data_type  \
profile                                                                    
3133824840               mpi         18         128          2    random   

            size_of_data_type scalability  
profile                                    
3133824840                  4      strong
```

#### Sample Sort Metadata
```
   cali.caliper.version  mpi.world.size  \
profile                                           
3133824840               2.11.0               2   

                                                 spot.metrics  \
profile                                                         
3133824840  min#inclusive#sum#time.duration,max#inclusive#...   

           spot.timeseries.metrics  spot.format.version  \
profile                                                   
3133824840                                            2   

                                                 spot.options  spot.channels  \
profile                                                                        
3133824840  time.variance,profile.mpi,node.order,region.co...  regionprofile   

           cali.channel spot:node.order   spot:output spot:profile.mpi  \
profile                                                                  
3133824840         spot            true  p2-a128.cali             true   

           spot:region.count spot:time.exclusive spot:time.variance  \
profile                                                               
3133824840              true                true               true   

              algorithm programming_model  group_num  input_size  num_procs  \
profile                                                                       
3133824840  Sample Sort               mpi         18         128          2   

           data_type  size_of_data_type scalability  
profile                                              
3133824840    random                  4      strong  

```

#### Radix Sort Metadata
```
           cali.caliper.version  mpi.world.size  \
profile                                           
3133824840               2.11.0               2   

                                                 spot.metrics  \
profile                                                         
3133824840  min#inclusive#sum#time.duration,max#inclusive#...   

           spot.timeseries.metrics  spot.format.version  \
profile                                                   
3133824840                                            2   

                                                 spot.options  spot.channels  \
profile                                                                        
3133824840  time.variance,profile.mpi,node.order,region.co...  regionprofile   

           cali.channel spot:node.order   spot:output spot:profile.mpi  \
profile                                                                  
3133824840         spot            true  p2-a128.cali             true   

           spot:region.count spot:time.exclusive spot:time.variance  \
profile                                                               
3133824840              true                true               true   

            launchdate                                          libraries  \
profile                                                                     
3133824840  1729137630  [/scratch/group/csce435-f24/Caliper/caliper/li...   

                                cmdline cluster algorithm programming_model  \
profile                                                                       
3133824840  [./radix_sort, --size, 128]       c     radix               mpi   

           data_type  size_of_data_type  input_size input_type  num_procs  \
profile                                                                     
3133824840       int                  4         128     Random          2   

           scalability  group_num implementation_source  
profile                                                  
3133824840      strong         18                online  
```

#### Bitonic Sort Metadata
```
cali.caliper.version  mpi.world.size  \
profile                                           
1981606483               2.11.0              16   

                                                 spot.metrics  \
profile                                                         
1981606483  min#inclusive#sum#time.duration,max#inclusive#...   

           spot.timeseries.metrics  spot.format.version  \
profile                                                   
1981606483                                            2   

                                                 spot.options  spot.channels  \
profile                                                                        
1981606483  time.variance,profile.mpi,node.order,region.co...  regionprofile   

           cali.channel spot:node.order   spot:output spot:profile.mpi  \
profile                                                                  
1981606483         spot            true  p16-a16.cali             true   

           spot:region.count spot:time.exclusive spot:time.variance  \
profile                                                               
1981606483              true                true               true   

            launchdate                                          libraries  \
...

           scalability  group_num implementation_source  
profile
1981606483      weak          1           online  
```                                        

## 4. Performance evaluation

## 4a. Bitonic Sort
### Strong Scaling
#### Main
![strong_scaling_65536_main](https://github.com/user-attachments/assets/29d6d89d-4720-4867-9e87-08fd9d634f1a)
![strong_scaling_262144_main](https://github.com/user-attachments/assets/dab494ec-5d1a-473d-91e7-7f87ce08023f)
![strong_scaling_1048576_main](https://github.com/user-attachments/assets/f23a5cc2-9154-49df-ae92-f882d962dc22)
![strong_scaling_4194304_main](https://github.com/user-attachments/assets/d621c3f0-5aa8-42b8-9c4b-76c5126d843e)
![strong_scaling_16777216_main](https://github.com/user-attachments/assets/636fb689-5233-46d2-9cc0-137fc3133526)
![strong_scaling_67108864_main](https://github.com/user-attachments/assets/15231001-1999-4ca5-97fc-7c8953b6d905)
![strong_scaling_268435456_main](https://github.com/user-attachments/assets/f157b419-c540-4b48-9716-4308e1b32b24)


#### Comp
![strong_scaling_65536 0_comp](https://github.com/user-attachments/assets/3e00ad2a-abb2-469b-80fe-a911c11d7092)
![strong_scaling_262144 0_comp](https://github.com/user-attachments/assets/de701558-b944-47d6-a79d-c28359066220)
![strong_scaling_1048576 0_comp](https://github.com/user-attachments/assets/1daa6a31-2a04-4eea-831d-1a5c294d21b6)
![strong_scaling_4194304 0_comp](https://github.com/user-attachments/assets/f6f72fc3-5587-4bd1-a981-7ea98fcda7c6)
![strong_scaling_16777216 0_comp](https://github.com/user-attachments/assets/abbb5039-0485-499a-b547-f938af69aea3)
![strong_scaling_67108864 0_comp](https://github.com/user-attachments/assets/3d2d8ca6-c30b-4c91-be09-911cede00f25)
![strong_scaling_268435456 0_comp](https://github.com/user-attachments/assets/55016de5-fa9a-432b-a8f0-75275abf2d5c)

Analysis:
The strong scaling plots show how the execution time more or less decreases as the number of processors increases while keeping the problem size constant for sizes 2^16 through 2^28. There are outliers that skew the data, but overall it seems to trend to be decreasing in time. Ideally, the execution time should decrease proportionally with the increase in the number of processors. With these plots, the strong scaling curve shows diminishing returns as more processors are added. This is due to overhead from inter-processor communication, synchronization, and non-parallelizable portions of the algorithm (as per Amdahl's law). It is worth noting that the time reduces much more with large problem sizes, meaning that parallelization is more effective and has more returns with a larger problem size. For larger problem sizes, the actual computational work increases significantly, and the overhead becomes a smaller fraction of the total work. The efficiency of parallelization improves with larger problem sizes, because the computational gains outweigh the overhead.
For the comp graphs, it is clear that the times decreased linearly consistently with the increase in the number of processes. The underlying bitonic algorithm used for comp scales well with the number of processes. The computational workload increases enough to offset the overhead of managing multiple processes, as with comp, where the algorithm is designed to take full advantage of the parallel architecture, minimizing any bottlenecks that might otherwise prevent linear scaling. The graphs stayed consistent for essentially all of the input sizes and types up until the larger inputs around 2^26 (I would consider those outliers). 



### Strong Speed-Up
#### Main
![speedup_sorted](https://github.com/user-attachments/assets/668b61c5-a397-4cf8-9247-ac013e29ef73)
![speedup_reverse_sorted](https://github.com/user-attachments/assets/517dc11a-0460-4a28-b09d-fb109ebd11a3)
![speedup_random](https://github.com/user-attachments/assets/c1c86fea-822d-4270-97c0-e62eedf57ce1)
![speedup_perturbed](https://github.com/user-attachments/assets/20bc8f8b-b6c5-41cf-9660-c8d6289a5209)


Analysis:
The plots overall show a decrease in speedup with more processes. The decrease in speedup is likely due to growing communication overhead, as more processes require more data exchanges, and synchronization delays increase. Amdahl's Law plays a role by limiting the speedup as non-parallelizable parts of the algorithm, like merging and comparing, become bottlenecks. Load imbalances between processes, increased cache contention, and memory bandwidth issues could also further reduce efficiency. The communication overhead can start to dominate the benefits of parallelization, especially as the number of processes grows. Therefore, the speedup indicates that added parallelization with more processes speeds up the time for the algorithm. it is worth noting that the different input sizes and input types do not seem to greatly impact the speedup trends.  

### Weak Scaling
![main_weak_scaling_sorted](https://github.com/user-attachments/assets/4cb8baa2-8035-4345-abdf-c77734ac8e7c)
![main_weak_scaling_reverse_sorted](https://github.com/user-attachments/assets/52eff17a-bbb2-4f97-90ef-5d162a3e7aac)
![main_weak_scaling_random](https://github.com/user-attachments/assets/4c149c8d-15ca-4798-895e-2c55eb5759bd)
![main_weak_scaling_perturbed](https://github.com/user-attachments/assets/3c13bb4b-3b12-46e7-b534-0a72d4e3a0a0)

Analysis:
It is observed that the average time per rank increases for the smaller problem sizes and decreases for the larger problem sizes - the scaling of the graphs makes the difference seem relatively small. This could be explained by the fact that for smaller problem sizes, the communication overhead becomes more significant compared to the actual computation each process has to perform. Bitonic sort involves frequent communication between processes, so smaller inputs don't have enough computational work to offset the communication time. As a result, the average time per rank increases because the overhead dominates the computation. But for larger problem sizes, the amount of computation per process increases significantly, which helps amortize the communication overhead. The processes spend more time on computation relative to communication, leading to a decrease in the average time per rank. In addition, with small inputs, adding more processes doesn’t fully utilize the available resources, meaning that some processes may remain idle or underused. The increased number of processes introduces unnecessary communication and synchronization overhead for small workloads, resulting in increased time per rank. For larger inputs, however, the workload scales with the number of processes, making each process more efficiently utilized, which lowers the average time per rank. it is clear that the times for 128 processes seem to be an outlier for all of the graphs. Before 128 processes, the lines tend to trend downward, indicating benefits from parallelization. This might be explained by bad data/outliers, or the fact that the communication overhead begins to overshadow the benefits. 

### Overall Analysis
Based on the resulting plots, the performance of the bitonic sort algorithm shows a balance between parallel efficiency and the inherent challenges of communication and synchronization. The algorithm benefits from parallelization, particularly with larger problem sizes, where computation dominates and overheads like communication are amortized, leading to near-linear reductions in execution time as the number of processes increases. This is shown in the consistent linear decrease in comp_large times, indicating that the algorithm scales well for large datasets.
However, the algorithm has diminishing returns as more processes are added, especially with smaller problem sizes. This is because communication overhead and synchronization begin to dominate, as shown by the decreasing speedup with additional processes. Amdahl’s Law limits the potential speedup due to the non-parallelizable parts of the algorithm, such as merging and comparing, which require extensive communication between processes. Additionally, random inputs make communication overhead worse due to unpredictable data movement, while the other input types lead to more efficient execution due to more predictable data patterns.
In weak scaling, the average time per rank initially increases for smaller problem sizes due to communication overhead, but decreases with larger problems as processes are more efficiently utilized. This indicates that while the bitonic sort algorithm can handle large-scale parallelism well, its performance is limited by communication costs and load imbalances at smaller scales.
Overall, the parallelized bitonic sort algorithm performs well for large problem sizes with high computation-to-communication ratios but struggles with communication overhead and diminishing returns as the number of processes increases, especially for smaller inputs. 


## 4b. Merge Sort

### Weak Scaling
![image](https://github.com/user-attachments/assets/6a09d57f-a6c9-4e8c-9c9b-21afb6f2511f)
![image](https://github.com/user-attachments/assets/fc7438b2-9eb9-41b0-a830-3266ce39f0f5)
![image](https://github.com/user-attachments/assets/9310cb5a-d70d-41a9-ae8a-2fd40248e573)

The three graphs represent weak scaling performance for communication (Comm), computation (Comp), and the main workload, all using sorted input data. As the number of threads increases, the speedup generally declines across all categories, highlighting the impact of overhead on scaling efficiency. Communication is the most affected, with a steep drop in performance, indicating that the overhead associated with communication becomes increasingly limiting as more threads are added. In contrast, computation maintains a more gradual decline, suggesting it scales more effectively and is less sensitive to the increase in threads. The main workload, which combines both communication and computation, shows a balanced trend with an intermediary decline in speedup, reflecting the cumulative impact of both types of overhead. Overall, these graphs reveal that communication overhead poses the greatest challenge to weak scaling, while computation retains relatively better efficiency.

### Speed Up
![image](https://github.com/user-attachments/assets/8c8ef3c5-f698-44ec-ac6b-09e705fb8dd2)
![image](https://github.com/user-attachments/assets/2ff53b68-59e6-443a-816d-c2e097f809a8)
![image](https://github.com/user-attachments/assets/8343d32e-dc21-4dae-af05-6ea754183c88)


The three graphs display speedup trends for weak scaling across different categories—communication (Comm), computation (Comp), and the main workload—with sorted data input. In the communication graph, speedup reaches a peak with a moderate number of threads but declines sharply as thread counts increase, suggesting significant communication overhead in handling larger thread numbers. The computation graph, however, shows a much higher peak speedup, especially for larger input sizes, indicating that computation benefits substantially from increased parallelism up to a certain point before diminishing returns set in. The main workload graph combines aspects of both communication and computation, exhibiting a moderate peak followed by a steady decline as threads increase. This mixed behavior reflects the balance between computation gains and communication costs, with the latter limiting overall efficiency at higher thread counts. Overall, while computation shows the highest scalability, communication becomes a bottleneck as thread count grows, impacting the main workload's speedup performance.

### Strong Scaling
![image](https://github.com/user-attachments/assets/11890515-7163-44c5-804b-796cd42a57c0)
![image](https://github.com/user-attachments/assets/22388a47-fee2-4b61-ac6c-95eb9b144c21)
![image](https://github.com/user-attachments/assets/517e8e3b-c7a0-419b-a039-095c6087137d)

The three graphs show strong scaling performance across different components—main, communication (comm), and computation (comp)—for an input size of \(2^16\). In the main workload graph, the average time per rank steadily increases as the number of processes rises, indicating limited scalability and suggesting that adding more processes doesn’t significantly improve performance. In contrast, the communication graph initially shows a decline in average time per rank, but after reaching a minimum point, it rises sharply with more processes, indicating that communication overhead increases significantly, especially beyond \(2^5\) processes. This suggests that, in this setup, communication efficiency deteriorates with higher process counts. The computation graph presents the most favorable scaling, with a distinct U-shaped curve; time per rank decreases as processes increase until around \(2^6\), after which it begins to rise slightly. This indicates that computation benefits from parallelism up to a point, but further increases in processes lead to diminishing returns, possibly due to synchronization overhead or resource contention. Overall, these graphs highlight the limitations of strong scaling, especially for communication-heavy tasks, while computation retains better scalability.

### Overal Analysis
In my parallel implementation of Merge Sort, I observed that computation scaled efficiently up to a certain point, with the average time per rank decreasing as the number of processes increased, indicating effective load distribution. However, communication overhead became a limiting factor as the process count grew, significantly impacting performance and hindering scalability. This suggests that while the computational aspects of my implementation benefit from parallelism, further optimization of communication could enhance overall performance and scalability.

## 4c. Sample Sort
### Strong Scaling
#### Main
![main_$2^{16}$_strong_scaling (3)](https://github.com/user-attachments/assets/81d3d415-d58d-4084-b318-d6706e5f1251)
![main_$2^{26}$_strong_scaling (2)](https://github.com/user-attachments/assets/399e5ca1-e169-44c5-b185-9cf28ff97eba)

In the above two plots we observe that when the sample size is small there's an increase in time to process the whole program, but when the sample size is large there's actually an decrease in the time when we increase the no. of processors. So, we see an better strong scaling for larger sample size.

#### Comm
![comm_$2^{16}$_strong_scaling (5)](https://github.com/user-attachments/assets/5dafc823-2b5f-4773-8faf-3b7ce5be4686)
![comm_$2^{26}$_strong_scaling](https://github.com/user-attachments/assets/d341fe55-2e79-4a4a-8912-fc1a2299391f)

We observe similar trend in our communication time, where when sample size is small we observe an increase in communication time but when sample size is large we observe an decrease in communication time when we increase the no. of processors. which says that parallelism helped our sample sort algorithm run efficiently and faster.

#### Comp
![comp_$2^{16}$_strong_scaling (3)](https://github.com/user-attachments/assets/d6232b2a-ce85-4fd8-a0e6-5f6f0615d8a1)
![comp_$2^{26}$_strong_scaling](https://github.com/user-attachments/assets/83f3b24d-c122-4d3e-be6d-b98805b8f55b)

Regardless of our sample size, we observe an decrease in computation time when we increase the no. of processors which implies our sample sort scales.


### Strong Speed-Up
#### Main
![speedup_plot_r_main](https://github.com/user-attachments/assets/e79bec09-1d20-4358-bb19-b36956ce06c1)

We observe the strongest speed-up from larger sample size, particularly when our sample size was 2^26 the algo experienced drastic speed-up until 2^6 processor after which the speed-up decreased

#### Comm
![speedup_plot_r_comm](https://github.com/user-attachments/assets/cd38044f-d130-485a-baca-068aa5c52d77)

We observe the strongest speed-up when our sample size is the largest while for smaller sample size we observe that the speed-up decreases as we increase the no. of processors.


#### Comp
![speedup_plot_r_comp](https://github.com/user-attachments/assets/b6d7e36e-2486-4871-a19d-ead1638400ad)

Regardless of the sample size we observe strong speed-up as we increase the no. of processors.

### Weak Scaling
#### main
![weak_scaling_main_random](https://github.com/user-attachments/assets/a3a03494-25fa-4646-86a3-6d6e78643bf3)

#### Comm
![weak_scaling_comm_random](https://github.com/user-attachments/assets/e4c404d9-d858-483b-80ea-891fd5ee3169)

#### Comp
![weak_scaling_comp_random](https://github.com/user-attachments/assets/64805672-e2e6-4ddf-a89b-e1b76e0211be)

In all above plots we observe an decreasing trend when we increase both the no. of processor and sample.


### Overall Analysis
Our sample sort algo scales efficiently and is helped by the parallelism. which can be observed particularly well in our strong scaling plots where we observed decreasing computation time when we increase the no. of processor for all sample size. But the sample size did matter for both overall and communication time, where we observed that when sample size is small we don't get the benefits of parallelism but when our sample size is larger we observed improvement in both the overall and communciation time. 




## 4d. Radix Sort
### Strong Scaling

![comm28](https://github.com/user-attachments/assets/6391444b-bdf8-4309-a21d-3b18f0d33d1a)

![comp_large28](https://github.com/user-attachments/assets/5cd9f207-414e-4c82-b27a-23b8ed99232f)

![main28](https://github.com/user-attachments/assets/2a4a73fa-b952-4ea3-bdb8-03add6cb438b)

### Strong Scaling Speed-Up

![speedup_comm_data_type_Random](https://github.com/user-attachments/assets/9a9cb0ab-1b80-4c79-83a4-bb23122ce131)

![speedup_comp_data_type_Random](https://github.com/user-attachments/assets/4be80fec-2487-418b-9ad7-e6dc58348c03)

![speedup_main_data_type_Random](https://github.com/user-attachments/assets/f84a8acb-a224-4292-83f9-dbf28eda93ad)

### Weak Scaling 

![comm_random](https://github.com/user-attachments/assets/62a91cfb-e647-45d1-839b-9d67c03cc6fe)

![comp_random](https://github.com/user-attachments/assets/552d3051-ba82-4144-843d-ae9d8fa664f2)

![main_random](https://github.com/user-attachments/assets/f63a8c5f-eb76-4431-bfbe-8b4d6ac2cd4a)


### Analysis
Radix Sort Overall Performance: The plots showed behaviors mostly in line with behaviors observed in previous labs when working with different input sizes and processor counts. This time however, there were different input types, and they mostly had impacts on communication. As input sizes increased, communication times often took longer. As processor counts increased, computation times often took less time. There were some kinks that could be worked out to improve the Caliper data, but this is mostly representative of the true data.

Notes: Due to the networking issues that many other students were facing, I was unable to successfully produce runs for 1024 processors. I was able to produce a few, which can be seen in a few of the plots above, but most other runs would hang and fail. To save credits from being unnecessarily used, I opted to just not run these and to possibly try over the weekend when there are not as many users.


## Comparing All Sorting Algoriths
### Input Type: Main & Random
![image](https://github.com/user-attachments/assets/b040c614-bb94-4c9e-9aa0-06e064e63b7e)
![image](https://github.com/user-attachments/assets/d495185b-44cd-41a3-958e-0c593596e37a)
![image](https://github.com/user-attachments/assets/25a6c8c8-c593-476f-a26c-c44895781a5b)
![image](https://github.com/user-attachments/assets/10475f7d-c358-4572-a73e-e78381408db5)

### Overal Analysis
The graphs display the performance of Bitonic Sort, Radix Sort, Sample Sort, and Merge Sort across different processor counts for random input data sizes \(2^16\) and \(2^26\). In terms of total execution time, Bitonic Sort and Sample Sort show strong scalability with increasing processors, maintaining low execution times, while Merge Sort and Radix Sort see significant time increases, especially at larger input sizes. For speedup, Sample Sort exhibits the highest initial gains, particularly at \(2^26\), but its speedup declines as the number of processors increases, suggesting inefficiency at higher scales. Bitonic Sort also achieves moderate speedup, though with a less dramatic initial peak. Both Radix Sort and Merge Sort have minimal speedup gains, indicating limited scalability in a parallel setting for this random input type. Overall, Bitonic and Sample Sort perform more efficiently in parallel on random data, while Radix and Merge Sort struggle to benefit from added processors, particularly with larger inputs, showing limitations in their parallel scalability.
