#include <mpi.h>
#include <iostream>
#include <vector>
#include <algorithm>
#include <adiak.hpp>
#include <caliper/cali.h>  // For Caliper instrumentation
#include <cstdlib>         // For std::srand and std::rand

// Function to merge two sorted arrays
void merge(std::vector<int>& arr, int left, int mid, int right) {
    int n1 = mid - left + 1;
    int n2 = right - mid;

    std::vector<int> L(n1);
    std::vector<int> R(n2);

    for (int i = 0; i < n1; ++i)
        L[i] = arr[left + i];
    for (int i = 0; i < n2; ++i)
        R[i] = arr[mid + 1 + i];

    int i = 0, j = 0, k = left;

    while (i < n1 && j < n2) {
        if (L[i] <= R[j]) {
            arr[k] = L[i];
            i++;
        } else {
            arr[k] = R[j];
            j++;
        }
        k++;
    }

    while (i < n1) {
        arr[k] = L[i];
        i++;
        k++;
    }

    while (j < n2) {
        arr[k] = R[j];
        j++;
        k++;
    }
}

// Sequential Merge Sort function
void mergeSort(std::vector<int>& arr, int left, int right) {
    if (left < right) {
        int mid = left + (right - left) / 2;

        mergeSort(arr, left, mid);
        mergeSort(arr, mid + 1, right);

        merge(arr, left, mid, right);
    }
}

int main(int argc, char* argv[]) {
    MPI_Init(&argc, &argv);

    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);  // Get current process rank
    MPI_Comm_size(MPI_COMM_WORLD, &size);  // Get number of processes

    // Initialize Adiak for metadata collection
    adiak::init(NULL);
    adiak::value("algorithm", "merge");
    adiak::value("programming_model", "mpi");
    adiak::value("group_num", 18);

    int n = 1000000;  // Total number of elements (example: 1 million)
    std::vector<int> data;

    // Master process initializes the data
    if (rank == 0) {
        data.resize(n);
        std::srand(42);  // Seed the random number generator
        // Generate random numbers to sort
        for (int i = 0; i < n; ++i) {
            data[i] = std::rand() % 100000;
        }
    }

    // Broadcast the size of the array
    MPI_Bcast(&n, 1, MPI_INT, 0, MPI_COMM_WORLD);

    // Determine the size of each local array
    int local_n = n / size;
    int remainder = n % size;
    std::vector<int> local_data(local_n + (rank < remainder ? 1 : 0));

    // Scatter the data to all processes
    std::vector<int> sendcounts(size);
    std::vector<int> displs(size);
    for (int i = 0; i < size; ++i) {
        sendcounts[i] = local_n + (i < remainder ? 1 : 0);
        displs[i] = i * local_n + std::min(i, remainder);
    }

    CALI_MARK_BEGIN("comm");
    MPI_Scatterv(data.data(), sendcounts.data(), displs.data(), MPI_INT, local_data.data(), local_data.size(), MPI_INT, 0, MPI_COMM_WORLD);
    CALI_MARK_END("comm");

    // Each process sorts its local array
    CALI_MARK_BEGIN("comp");
    std::sort(local_data.begin(), local_data.end());
    CALI_MARK_END("comp");

    // Gather the sorted sub-arrays back to the master process
    CALI_MARK_BEGIN("comm_large");
    MPI_Gatherv(local_data.data(), local_data.size(), MPI_INT, data.data(), sendcounts.data(), displs.data(), MPI_INT, 0, MPI_COMM_WORLD);
    CALI_MARK_END("comm_large");

    // Master process merges the sorted sub-arrays
    if (rank == 0) {
        CALI_MARK_BEGIN("comp_large");
        for (int i = 1; i < size; ++i) {
            int mid = displs[i] - 1;
            int right = displs[i] + sendcounts[i] - 1;
            merge(data, 0, mid, right);
        }
        CALI_MARK_END("comp_large");

        std::cout << "Sorted array (first 10 elements): ";
        for (int i = 0; i < 10; ++i) {
            std::cout << data[i] << " ";
        }
        std::cout << std::endl;
    }

    // Collect metadata for the experiment
    adiak::value("input_size", n);
    adiak::value("num_procs", size);
    adiak::value("data_type", "random");
    adiak::value("size_of_data_type", sizeof(int));
    adiak::value("scalability", "strong");

    // Finalize Adiak and MPI
    //adiak::finalize();
    MPI_Finalize();

    return 0;
}