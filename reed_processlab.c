#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <time.h>


int main(int argc, char *argv[])
{

	if(argc != 3)
	{
		printf("Insufficient arguments.");
		return EXIT_FAILURE;
	}

	//initialize status variable for wait call in parent process
	//accepts input from user for number of processes
	int status;
	int numProcesses = atoi(argv[2]);
	
	//open file and determine file size - unsigned longs used for larger files
	int inFile;
	inFile = open(argv[1], O_RDONLY);
	struct stat f;
	fstat(inFile, &f);	
	unsigned long long int fileSize = f.st_size;

	//assign chunkSize as the quotient of fileSize and number of processes that divide the file
	unsigned long long int chunkSize = fileSize / numProcesses;

	//if the file size is not divisible by number of processes, add to the chunk size to ensure divisibility
	if(fileSize % numProcesses != 0)
	{
		chunkSize++;
	}

	//instantiate children array with size of number of processes
	//initialize both the children's sum (portion_sum) and parent sum (end_sum) as zero to start
	//initialize i, used for iterative number of processes
	pid_t children[numProcesses];
	int portion_sum = 0;
	int end_sum = 0;
	int i;

	//create packet of chunk size to be iterated over
	unsigned char packetBuffer[chunkSize];

	//instantiate clock with start and end times to check runtime
	clock_t start_t, end_t; 
	double total_t;
	start_t = clock();

	//create argv[2] number of children processes
	for(i = 0; i < numProcesses; i++)
	{
		//reading file within child allows next child process to pick up on last section of file
		chunkSize = read(inFile, packetBuffer, chunkSize);
		children[i] = fork();

		//return error if child pid is less than 0
		if(children[i] < 0)
		{
			printf("Error occurred.");
			return EXIT_FAILURE;
		}

		//if pid is 0, run bitwise XOR process
		if(children[i] == 0)
		{	
			int k;
			for(k = 0; k < chunkSize;k++)
			{
				//XOR bytes in packet to child's sum
				portion_sum ^= packetBuffer[k];
			}
			//return that child's sum to parent process
			return portion_sum; 
		}	

		//if pid is above 0, just continue through process
		if(children[i] > 0)
		{
			continue;
		}
	}
	//close file after use
	close(inFile);
	
	//for number of processes, run parent process to calculate final checksum
	int j;
	for(j = 0; j < numProcesses; j++)
	{
		//wait for child process to finish and accept return value
		wait(&status);
		if (WIFEXITED(status))
		{
			//XOR return value to final checksum
			end_sum ^= WEXITSTATUS(status);
		}	
	}
	//end runtime of process since it's only print statement from here
	end_t = clock();
	total_t = (double)(end_t - start_t) / CLOCKS_PER_SEC;

	//print out final runtime and checksum
	printf("Total time was %f seconds\n", total_t);
	printf("Final checksum: %d\n", end_sum);
	return end_sum;

	
	
}
