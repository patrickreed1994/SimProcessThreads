#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>


void *threadMaker(void *threadid)
{
	//define variable to hold thread id
	long tid;
	tid = (long)threadid;

	//quit thread after it is defined
	pthread_exit(NULL);
}

int threadmath(int t, int chunkSize, char packetBuffer[])
{
	//define variable for for loop initialization
	int i;
	//define checksum for portion of file that thread will read over
	int portion_sum = 0;
		//read over chunk and XOR to chunk's checksum
		for(i = 0; i < chunkSize;i++)
		{
			portion_sum ^= packetBuffer[i];
		}
	//return this thread's checksum for the chunk it iterated over
	return portion_sum;
}

int main(int argc, char *argv[])
{

	if(argc != 3)
	{
		printf("Insufficient arguments.");
		return EXIT_FAILURE;
	}
	//accept input for number of threads
	int numThreads = atoi(argv[2]);

	//create array of threads for use
	pthread_t threads[numThreads];

	//define initialization variables for for loops
	//rc define as pointer for pthread_create
	int rc;
	long t, i, j;

	//open file and determine file size - unsigned longs used for larger files
	int inFile = open(argv[1], O_RDONLY);
	struct stat f;
	fstat(inFile, &f);	
	long long fileSize = f.st_size;

	//assign chunkSize as the quotient of fileSize and number of processes that divide the file
	long chunkSize = fileSize / numThreads;

	//if the file size is not divisible by number of threads, 
	//add to the chunk size to ensure divisibility.
	if(fileSize % numThreads != 0)
	{
		chunkSize++;
	}

	//initialize checksum array to store thread XORs
	//initialize final_sum as value of all partial sums in checksum array
	int checksum[numThreads];
	int final_sum = 0;

	//buffer for threadmath to read from and iterative over
	unsigned char packetBuffer[chunkSize];
	
	//instantiate clock with start and end times to check runtime
	clock_t start_t, end_t; 
	double total_t;
	start_t = clock();

	//do threading process for the number of threads the user put as input
	for(t=0;t<numThreads;t++)
	{
		//create thread
		rc = pthread_create(&threads[t], NULL, threadMaker, (void  *)t);
		
		//point to chunk of data from inFile for use by threadmath function
		chunkSize = read(inFile, packetBuffer, chunkSize);

		//run threadmath function and add result to checksum array in position t
		checksum[t] = threadmath(t, chunkSize, packetBuffer);

		//in the event of error, return error code and exit function
		if(rc)
		{
			printf("Error: return code from pthread_create() is %d\n", rc);
			exit(-1);
		}

	}
	//close file after use
	close(inFile);

	//iterative over checksum array, add each element to final_sum
	for(j = 0; j < numThreads; j++)
	{
		final_sum ^= checksum[j];
	}
	//end runtime of process since it's only print statement from here
	end_t = clock();
	total_t = (double)(end_t - start_t) / CLOCKS_PER_SEC;

	//print out final runtime and checksum
	printf("Total time was %f seconds\n", total_t);
	printf("final checksum is %d\n", final_sum);
	//return final checksum
	return final_sum;
	//exit out of pthreading
	pthread_exit(NULL);
}
