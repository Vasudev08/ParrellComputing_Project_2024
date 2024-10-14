/******************************************************************************
* FILE: bitonic_sort.cpp
* DESCRIPTION:  
*   MPI Bitonic Sort in C++
*   In this code, the master task distributes a matrix multiply
*   operation to numtasks-1 worker tasks.
* AUTHOR: Anna Hartman
* LAST REVISED: 10/09/24
******************************************************************************/




/******************************************************************************
* PARALLEL
******************************************************************************/


#include <iostream>
#include <vector>
#include <algorithm>
#include <cstdlib> // For rand() and srand()
#include <ctime>   // For time()
#include <mpi.h>

void bitonicMerge(std::vector<int>& arr, int low, int count, bool dir) {
    if (count > 1) {
        int k = count / 2;
        for (int i = low; i < low + k; i++) {
            if (dir == (arr[i] > arr[i + k])) {
                std::swap(arr[i], arr[i + k]);
            }
        }
        bitonicMerge(arr, low, k, dir);
        bitonicMerge(arr, low + k, k, dir);
    }
}

void bitonicSort(std::vector<int>& arr, int low, int count, bool dir) {
    if (count > 1) {
        int k = count / 2;
        bitonicSort(arr, low, k, true);  // Sort in ascending order
        bitonicSort(arr, low + k, k, false); // Sort in descending order
        bitonicMerge(arr, low, count, dir); // Merge the result
    }
}

void parallelBitonicSort(std::vector<int>& arr, int size, int rank, int numProcs) {
    // Calculate local array size
    int localSize = size / numProcs;
    std::vector<int> localArr(localSize);

    // Scatter the array to all processes
    MPI_Scatter(arr.data(), localSize, MPI_INT, localArr.data(), localSize, MPI_INT, 0, MPI_COMM_WORLD);

    std::cout << "Process " << rank << " received: ";
    for (int i = 0; i < localSize; i++) {
        std::cout << localArr[i] << " ";
    }
    std::cout << std::endl;

    // Sort the local array
    bitonicSort(localArr, 0, localSize, true);
    std::cout << "Process " << rank << " sorted its part: ";
    for (int i = 0; i < localSize; i++) {
        std::cout << localArr[i] << " ";
    }
    std::cout << std::endl;

    // Gather sorted local arrays back to the root process
    MPI_Gather(localArr.data(), localSize, MPI_INT, arr.data(), localSize, MPI_INT, 0, MPI_COMM_WORLD);

    // Merge the sorted arrays in the root process
    if (rank == 0) {
        // Perform a bitonic merge for the whole array
        std::cout << "Root process is merging the sorted arrays." << std::endl;
        bitonicSort(arr, 0, size, true); // Sort the gathered array
    }
}

int main(int argc, char** argv) {
    MPI_Init(&argc, &argv);

    int rank, numProcs;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &numProcs);

    const int SIZE = 16; // Adjust size as necessary
    std::vector<int> arr;

    if (rank == 0) {
        // Seed the random number generator and create a random array
        srand(static_cast<unsigned int>(time(nullptr))); 
        arr.resize(SIZE);
        for (int i = 0; i < SIZE; i++) {
            arr[i] = rand() % 100; // Random numbers between 0 and 99
        }
        std::cout << "Initial array: ";
        for (int i : arr) std::cout << i << " ";
        std::cout << std::endl;
    }

    // Perform the parallel bitonic sort
    parallelBitonicSort(arr, SIZE, rank, numProcs);

    // Print the sorted array in the root process
    if (rank == 0) {
        std::cout << "Sorted array: ";
        for (int i : arr) std::cout << i << " ";
        std::cout << std::endl;
    }

    MPI_Finalize();
    return 0;
}