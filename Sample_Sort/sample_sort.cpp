/*
- Finally, each processor merges and sorts the recieved elements.

*/

#include <iostream>
#include <fstream>
#include <mpi.h>
#include <cmath> 
#include <caliper/caliper.h>


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

void merge_elements(int arr[], int counts[], int k) {
	int totalelements = 0;

	for (int i = 0; i < k; i++) {
		totalelements = totalelements + counts[i];
	}

	quickSort(arr, 0, totalelements - 1);
}


void print_array(int* array, int count) {
    std::cout << "Numbers read from the file: ";
    for (int i = 0; i < count; i++) {
        std::cout << array[i] << " ";
    }
    std::cout << std::endl;
}

void multiPivotPartition(int arr[], int n, int pivots[], int numPivots, int** segments) {
    int SegmentStartIndex = 0;

    for (int i = 0; i < numPivots; ++i) {
        int pivot = pivots[i];
        int segmentEndIndex = SegmentStartIndex;

        while (segmentEndIndex < n && arr[segmentEndIndex] <= pivot) {
            ++segmentEndIndex;
        }

        if (segmentEndIndex > SegmentStartIndex) {
            segments[i][0] = SegmentStartIndex;
            segments[i][1] = segmentEndIndex - 1;

        } else {
            segments[i][0] = segments[i][1] = -1;
        }

        SegmentStartIndex = segmentEndIndex;
    }

    if (SegmentStartIndex < n) {
        segments[numPivots][0] = SegmentStartIndex;
        segments[numPivots][1] = n - 1;

    } else {
        segments[numPivots][0] = segments[numPivots][1] = -1;
    }

}




int main(int agrc, char* argv[]) {
    caliper::init();
    CALIPER_MARK_REGION("main");
    if (agrc != 3) {
        std::cout << "Usage: Please enter input filename to perform sorting.\n";
        return 1;
    }

    std::ifstream input(argv[1]);

    if (!input) {
        std::cout << "Cannot open input file.\n";
        return 1;
    }

    int n;
	sscanf(argv[2], "%d", &n);	// Total number of elements

    int numProcess, processId;

    double startTime;

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

    CALIPER_MARK_REGION("data_init_io");

    if (processId == MASTER) {
        sendBuffer = (int*)malloc(n * sizeof(int));
        int arraySize = read_file(input, sendBuffer);
        cout << "Number of elements read: " << arraySize << endl; // Check read count



    }

    startTime = MPI_Wtime();
    MPI_Scatter(sendBuffer, nPerProcess, MPI_INT, receiveBuffer, nPerProcess, MPI_INT, 0, MPI_COMM_WORLD);
    CALIPER_END_REGION("data_init_io");


    //Run basic sorting algorithm on each of the segments, where each processor is handling its piece independently.
    CALIPER_MARK_REGION("comp");
    quickSort(receiveBuffer, 0, nPerProcess-1);
    CALIPER_END_REGION("comp");



    //From these sorted segments, each processor selects few samples. then, MPI communication is used to collect samples from all processors
    CALIPER_MARK_REGION("comm");

    int sampleSelectThresold = n / (numProcess * numProcess); // elements to skip for selecting pivots
    int* processSamples = (int*)malloc(numProcess * sizeof(int));
    for (int i = 0, j = 0; i < nPerProcess; i = i + sampleSelectThresold) {
        processSamples[j] = receiveBuffer[i];
        j++;
    }

    int* receiveBufferProcessSamples = (int*)malloc(numProcess * numProcess * sizeof(int));
    MPI_Gather(processSamples, numProcess, MPI_INT, receiveBufferProcessSamples, numProcess, MPI_INT, 0, MPI_COMM_WORLD);
    CALIPER_END_REGION("comm");


    int* pivotsSelected = (int*)malloc((numProcess - 1) * sizeof(int));

    //Sort the selected samples, which will help us establish a global order among the samples.
    //From the sorted samples, we pick few speical elements. which act as a pivot. which are shared with all processors. MPI_Bcast is used to broadcast the pivots to all processors.

    if (processId == MASTER) {
        CALIPER_MARK_REGION("comp");

        quickSort(receiveBufferProcessSamples, 0, (numProcess * numProcess) - 1);

        int pivotSelectThresold = numProcess + (int)floor(numProcess / 2.0) - 1;
        for (int i = pivotSelectThresold, j = 0; i < numProcess * numProcess; i = i + numProcess ) {
            pivotsSelected[j] = receiveBufferProcessSamples[i];
            j++;
        }
        CALIPER_END_REGION("comp");

    }

    MPI_Bcast(pivotsSelected, numProcess - 1, MPI_INT, 0, MPI_COMM_WORLD);


    // Each processor takes its ordered segment and divides it into subsegments based on the choicen pivot.

    int numPivotSection = numProcess - 1;
    int maxSegments = numPivotSection + 1;

    int** segments;
    segments = (int**)malloc(maxSegments * sizeof(int*));
    for (int i = 0; i < maxSegments; i++) {
        segments[i] = (int*)malloc(2 * sizeof(int));
    }

    multiPivotPartition(receiveBuffer, nPerProcess, pivotsSelected, numPivotSection, segments);


    // Now, processors share their ordered segments globally with the corresponding processor based on the segment number. using MPI_Alltoall
    CALIPER_MARK_REGION("comm");
    int* partitionSizes = (int*)malloc((numProcess - 1) * sizeof(int));

    for (int i = 0; i < maxSegments; ++i) {
        int start = segments[i][0];
        int end  = segments [i][1];

        if (start != -1 && end != -1) {
            partitionSizes[i] = end - start + 1;
        } else {
            partitionSizes[i] = 0;
        }
    }

    int* sendDispIndex = (int*)malloc(numProcess * sizeof(int));
    int* recvDispIndex = (int*)malloc(numProcess * sizeof(int));
    MPI_Barrier(MPI_COMM_WORLD);

    int* newPartitionSizes = (int*)malloc((numProcess - 1) * sizeof(int));
    MPI_Alltoall(partitionSizes, 1, MPI_INT, newPartitionSizes, 1, MPI_INT, MPI_COMM_WORLD);

    int totalSize = 0;
    for (int i = 0; i < numProcess; i ++) {
        totalSize += newPartitionSizes[i];
    }


    int* newPartitions = (int*)malloc(totalSize * sizeof(int));
	//*newPartitions = (double*)malloc(totalSize * sizeof(double));

	sendDispIndex[0] = 0;
	recvDispIndex[0] = 0; //Calculate the displacement relative to recvbuf, this displacement stores the data received from the process
	for (int i = 1; i < numProcess; i++) {
		sendDispIndex[i] = partitionSizes[i - 1] + sendDispIndex[i - 1];
		recvDispIndex[i] = newPartitionSizes[i - 1] + recvDispIndex[i - 1];
	}
	MPI_Barrier(MPI_COMM_WORLD);

	MPI_Alltoallv(receiveBuffer, partitionSizes, sendDispIndex, MPI_INT, newPartitions, newPartitionSizes, recvDispIndex, MPI_INT, MPI_COMM_WORLD);
    CALIPER_END_REGION("comm");


    CALIPER_MARK_REGION("comp")
    
    merge_elements(newPartitions, newPartitionSizes, numProcess);
    CALIPER_END_REGION("comp")


    CALIPER_MARK_REGION("comm");
    int totalelements = 0;

    for (int i = 0; i < numProcess; i++) {
        totalelements = totalelements + newPartitionSizes[i];
    }

    int* subListSizes = (int*)malloc(numProcess * sizeof(int));
	int* recvDisp2 = (int*)malloc(numProcess * sizeof(int));

	MPI_Gather(&totalelements, 1, MPI_INT, subListSizes, 1, MPI_INT, 0, MPI_COMM_WORLD);


    if (processId == 0) {
        recvDisp2[0] = 0;
        for (int i =  1; i < numProcess; i++) {
			recvDisp2[i] = subListSizes[i - 1] + recvDisp2[i - 1];
        }
    }

    MPI_Barrier(MPI_COMM_WORLD);
	int* last_array_sorted = (int*)malloc(n * sizeof(int));
	//Send each sorted sublist back to the root process
	MPI_Gatherv(newPartitions, totalelements, MPI_INT, last_array_sorted, subListSizes, recvDisp2, MPI_INT, 0, MPI_COMM_WORLD);

	MPI_Barrier(MPI_COMM_WORLD);
    CALIPER_END_REGION("comm");


    if (processId == 0) {

		FILE* file = fopen("output.txt", "w");
		if (file != NULL) {
			for (int i = 0; i < n; i++) {
				fprintf(file, "%d ", last_array_sorted[i]);
			}
			fclose(file);
		}
		else {
			fprintf(stderr, "Error opening file.\n");
		}
    }
		
        
    double elapsedTime = MPI_Wtime() - startTime;
	if (processId == MASTER) {
		printf("Total Elapsed time during the process : %f\n", elapsedTime);
	}

	MPI_Finalize();
	return 0;


}