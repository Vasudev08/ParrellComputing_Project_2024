/*
In sample sort a large dataset is divided into smaller paritions, after which each partition is sorted independently, and then these sorted paritions are merged to obtain the final sorted result.

- Run basic sorting algorithm on each of the segments, where each processor is handling its piece independently.
- From these sorted segments, each processor selects few samples. then, MPI communication is used to collect samples from all processors
- Sort the selected samples, which will help us establish a global order among the samples.
- From the sorted samples, we pick few speical elements. which act as a pivot. which are shared with all processors. MPI_Bcast is used to broadcast the pivots to all processors.
- Each processor takes its ordered segment and divides it into subsegments based on the choicen pivot.
- Now, processors share their ordered segments globally with the corresponding processor based on the segment number. using MPI_Alltoall
- Finally, each processor merges and sorts the recieved elements.

*/

#include <iostream>
#include <fstream>
#include <mpi.h>


using namespace std;

#define MASTER 0

int read_file(std::ifstream& lineInput, int* array) {
    int i = 0;
    while (lineInput >> array[i]) {
        i++;
    }

    return i;

}

void print_array(int* array, int count) {
    std::cout << "Numbers read from the file: ";
    for (int i = 0; i < count; i++) {
        std::cout << array[i] << " ";
    }
    std::cout << std::endl;
}


int main(int agrc, char* argv[]) {
    if (agrc != 2) {
        std::cout << "Usage: Please enter input filename to perform sorting.\n";
        return 1;
    }

    std::ifstream input(argv[1]);

    if (!input) {
        std::cout << "Cannot open input file.\n";
        return 1;
    }

    const int MAX_SIZE = 100; // You can adjust this size based on your input
    int array[MAX_SIZE];

    int count = read_file(input, array);

    print_array(array, count);

    input.close();

    int n;
    n = count;

    int numProcess, processId;

    double startTime, endTime;

    //Splitting the given dataset into equally divided smaller segments, where each segement is given to a processor
    
    MPI_Init(&agrc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &numProcess);
    MPI_Comm_rank(MPI_COMM_WORLD, &processId); 
    printf("MPI process %d has started...\n", processId);






    return 0;


}