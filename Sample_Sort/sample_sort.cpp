/*
- Sort the selected samples, which will help us establish a global order among the samples.
- From the sorted samples, we pick few speical elements. which act as a pivot. which are shared with all processors. MPI_Bcast is used to broadcast the pivots to all processors.
- Each processor takes its ordered segment and divides it into subsegments based on the choicen pivot.
- Now, processors share their ordered segments globally with the corresponding processor based on the segment number. using MPI_Alltoall
- Finally, each processor merges and sorts the recieved elements.

*/

#include <iostream>
#include <fstream>
#include <mpi.h>

#define MASTER 0



using namespace std;

#define MASTER 0

int read_file(std::ifstream& lineInput, int* array) {
    int i = 0;
    while (lineInput >> array[i]) {
        i++;
    }

    return i;

}

int elementCount(std::ifstream& lineInput) {
    int count = 0;
    int temp;
    
    while (lineInput >> temp) {
        count++;
    }
    return count;
}

int partition(int arr[], int start, int end) {
    int pivot = arr[start];

    int count = 0;

    for (int i = start + 1; i<= end; i++) {
        if (arr[i] <= pivot) {
            count++;
        }
    }

    int pivotIndex = start + count;
	swap(arr[pivotIndex], arr[start]);

    // Sorting left and right parts of the pivot element
	int i = start, j = end;

	while (i < pivotIndex && j > pivotIndex) {

		while (arr[i] <= pivot) {
			i++;
		}

		while (arr[j] > pivot) {
			j--;
		}

		if (i < pivotIndex && j > pivotIndex) {
			swap(arr[i++], arr[j--]);
		}
	}

	return pivotIndex;

    
}

void quickSort(int arr[], int start, int end) {
    if (start >= end) {
        return;
    }

    int p = partition(arr, start, end);

    quickSort(arr, start, p - 1);
    quickSort(arr, p + 1, end);
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

    int count = elementCount(input, array);

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

    // the amount of intergers send to each processor
    int nPerProcess = n / numProcess;

    /*Scattering the arrary into difference processors via receiveBuffer*/
    //Allocating memory for the receive buffer
    int* receiveBuffer = (int*)malloc(nPerProcess * sizeof(int));

    int* sendBuffer = NULL;
    if (processId == MASTER) {
        sendBuffer = (int*)malloc(n * sizeof(int));

    }

    startTime = MPI_Wtime();
    MPI_Scatter(sendBuffer, nPerProcess, MPI_INT, receiveBuffer, nPerProcess, MPI_INT, 0, MPI_COMM_WORLD);


    //Run basic sorting algorithm on each of the segments, where each processor is handling its piece independently.
    quickSort(receiveBuffer, 0, nPerProcess-1);


    //From these sorted segments, each processor selects few samples. then, MPI communication is used to collect samples from all processors
    int pivotSelectThresold = n / (numProcess * numProcess); // elements to skip for selecting pivots
    int* processPivots = (int*)malloc(numProcess * sizeof(int));
    for (int i = 0, j = 0, i < nPerProcess; i = i + pivotSelectThresold) {
        processPivots[j] = receiveBuffer[i];
        j++
    }






    return 0;


}