
#include <iostream>
#include <vector>
#include <algorithm>
#include <cstdlib> // For rand() and srand()
#include <ctime>   // For time()
#include <mpi.h>
#include <caliper/cali.h>
#include <adiak.hpp>
#include <cmath>

// create input for bitonic sort
void generateInput(std::vector<int>& arr, int size, const std::string& input_type) {
    if (input_type == "sorted") {
        for (int i = 0; i < size; i++) {
            arr[i] = i;
        }
    } else if (input_type == "reverse_sorted") {
        for (int i = 0; i < size; i++) {
            arr[i] = size;
        }
    } else if (input_type == "random") {
        for (int i = 0; i < size; i++) {
            arr[i] = rand() % 100000; // Random numbers between 0 and 100000
        }
    } else if (input_type == "perturbed") {
        for (int i = 0; i < size; i++) {
            arr[i] = i;
        }
        int swaps = size * 0.01; // 1% perturbed
        for (int i = 0; i < swaps; i++) {
            int idx1 = rand() % size;
            int idx2 = rand() % size;
            std::swap(arr[idx1], arr[idx2]);
        }
    }
}


void bitonicMerge(std::vector<int>& arr, int low, int count, bool dir) {
    CALI_MARK_BEGIN("comp");
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
    CALI_MARK_END("comp");
}

void bitonicSort(std::vector<int>& arr, int low, int count, bool dir) {
    CALI_MARK_BEGIN("comp");
    if (count > 1) {
        int k = count / 2;
        bitonicSort(arr, low, k, true);  // Sort in ascending order
        bitonicSort(arr, low + k, k, false); // Sort in descending order
        bitonicMerge(arr, low, count, dir); // Merge the result
    }
    CALI_MARK_END("comp");
}

void parallelBitonicSort(std::vector<int>& arr, int size, int rank, int numProcs) {
    // Calculate local array size
    int localSize = size / numProcs;
    std::vector<int> localArr(localSize);

    // Scatter the array to all processes
    MPI_Scatter(arr.data(), localSize, MPI_INT, localArr.data(), localSize, MPI_INT, 0, MPI_COMM_WORLD);
   

    // std::cout << "Process " << rank << " received: ";
    // for (int i = 0; i < localSize; i++) {
    //     std::cout << localArr[i] << " ";
    // }
    // std::cout << std::endl;

    // Sort the local array
    bitonicSort(localArr, 0, localSize, true);
    // std::cout << "Process " << rank << " sorted its part: ";
    // for (int i = 0; i < localSize; i++) {
    //     std::cout << localArr[i] << " ";
    // }
    // std::cout << std::endl;

    // Gather sorted local arrays back to the root process
    MPI_Gather(localArr.data(), localSize, MPI_INT, arr.data(), localSize, MPI_INT, 0, MPI_COMM_WORLD);


    // Merge the sorted arrays in the root process
    if (rank == 0) {
        // Perform a bitonic merge for the whole array
        // std::cout << "Root process is merging the sorted arrays." << std::endl;
        bitonicSort(arr, 0, size, true); // Sort the gathered array
    }
}

// Function to check if the array is sorted
bool isSorted(const std::vector<int>& arr) {
    for (size_t i = 1; i < arr.size(); ++i) {
        if (arr[i] < arr[i - 1]) {
            return false; // Found an element that is out of order
        }
    }
    return true; // Array is sorted
}

int main(int argc, char** argv) {
   CALI_MARK_BEGIN("main");
   MPI_Init(&argc, &argv);

    int rank, numProcs;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &numProcs);

    // Ensure that two arguments (size and number of processes) are provided
    if (argc < 3) {
        if (rank == 0) {
            std::cerr << "Usage: " << argv[0] << " <size exponent> <number of processes>" << std::endl;
        }
        MPI_Finalize();
        return 1;
    }

    // Parse arguments
    int exponent = std::atoi(argv[1]);  // Get the exponent (size)
    int size = std::pow(2, exponent);   // Calculate size as 2^exponent
    int requestedProcs = std::atoi(argv[2]);  // Get the requested number of processes

    if (numProcs != requestedProcs) {
        if (rank == 0) {
            std::cerr << "Error: The number of processes specified (" << requestedProcs 
                      << ") does not match the number of processes started (" << numProcs << ")." << std::endl;
        }
        MPI_Finalize();
        return 1;
    }

    std::vector<int> arr;

    if (rank == 0) {
        // Seed the random number generator and create a random array
        srand(static_cast<unsigned int>(time(nullptr)));
        arr.resize(size);
        for (int i = 0; i < size; i++) {
            arr[i] = rand() % 10000; // Random numbers between 0 and 99
        }
        // std::cout << "Initial array: ";
        // for (int i : arr) std::cout << i << " ";
        // std::cout << std::endl;

        // Caliper metadata collection
        adiak::init(NULL);
        adiak::launchdate();
        adiak::libraries();
        adiak::cmdline();
        adiak::clustername();
        adiak::value("algorithm", "bitonic");
        adiak::value("programming_model", "mpi");
        adiak::value("data_type", "int");
        adiak::value("size_of_data_type", sizeof(int));
        adiak::value("input_size", size);
        adiak::value("input_type", "Random");
        adiak::value("num_procs", numProcs);
        adiak::value("scalability", "weak"); 
        adiak::value("group_num", 1); 
        adiak::value("implementation_source", "online"); 
    }

    // Perform the parallel bitonic sort
    CALI_MARK_BEGIN("comm");
    if (localSize <= 10) {  // Arbitrary small size for demonstration
        CALI_MARK_BEGIN("comm_small");
    } else {
        CALI_MARK_BEGIN("comm_large");
    }
    parallelBitonicSort(arr, size, rank, numProcs);

    CALI_MARK_END(localSize <= 10 ? "comm_small" : "comm_large");
    CALI_MARK_END("comm");
    

    // Print the sorted array in the root process
    if (rank == 0) {
        std::cout << "Sorted array: ";
        for (int i : arr) std::cout << i << " ";
        std::cout << std::endl;

        // Correctness check
        CALI_MARK_BEGIN("correctness_check");
        if (isSorted(arr)) {
            std::cout << "Array is sorted correctly." << std::endl;
        } else {
            std::cout << "Array is NOT sorted correctly!" << std::endl;
        }
        CALI_MARK_END("correctness_check");
    }

    CALI_MARK_END("main");
    MPI_Finalize();
    return 0;
}

