# CSCE 435 Group project

## 0. Group number:

## 1. Group members:

1. Anna Hartman
2. Tate Moskal
3. Nicole Hernandez
4. Vasudev Agarwal

## 2. Project topic (e.g., parallel sorting algorithms)

### 2a. Brief project description (what algorithms will you be comparing and on what architectures)

- Bitonic Sort:
- Sample Sort:
- Merge Sort: Parallel Merge Sort uses the divide and conquer technique, recursively dividing the dataset into smaller parts, sorting them, and merging the results where in parallel ver. it is distributed across multiple processors. Parallel Merge Sort will be implemented using MPI.
- Radix Sort:

### 2b. Pseudocode for each parallel algorithm

#### 2b.1 Bitonic Sort

#### 2b.2 Sample Sort

In sample sort a large dataset is divided into smaller paritions,
after which each partition is sorted independently, and then these
sorted paritions are merged to obtain the final sorted result.

STEP 1: Splitting the given dataset into equally divided smaller
segments, where each segement is given to a processor

STEP 2: Run basic sorting algorithm on each of the segments,
where each processor is handling its piece independently.

STEP 3: From these sorted segments, each processor selects
few samples. these samples give us the idea of overall list distribution

STEP 4: Sort the selected samples, which will help us establish a
global order among the samples.

STEP 5: From the sorted samples, we pick few speical elements. which act
as a pivot. which are shared with all processors

STEP 6: Each processor takes its ordered segment adn divides it into
subsegments based on the choicen pivot.

STEP 7: Now, processors share their ordered segments globally with the
corresponding processor based on the segment number.

STEP 8: Finally, each processor merges and sorts the recieved elements.

- For MPI programs, include MPI calls you will use to coordinate between processes

#### 2b.3 Merge Sort

1. Start MPI for Communication between processors.
2. Identify if the process is the master (rank 0) or worker (rank > 0)
3. The master splits the dataset into smaller parts
4. The master sends each part to a worker process
5. Each worker process sorts its part of the dataset independently
6. Workers send their sorted parts back to the master
7. The master merges all sorted parts into one sorted array
8. Close MPI after sorting is complete

#### 2b.4 Radix Sort

### 2c. Evaluation plan - what and how will you measure and compare

- Input sizes, Input types
- Strong scaling (same problem size, increase number of processors/nodes)
- Weak scaling (increase problem size, increase number of processors)
