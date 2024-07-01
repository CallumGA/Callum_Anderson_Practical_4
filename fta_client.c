#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <arpa/inet.h>

#define MAXLINE 128 // maximum bytes to receive and send at once

// External functions
extern int swap_open(unsigned int addr, unsigned short port);
extern int swap_write(int sd, unsigned char buf[], int length);
extern void swap_close(int sd);

int main(int argc, char *argv[])
{
	unsigned short server_port;
	unsigned int server_address;
	unsigned char buf[MAXLINE];
	int sd, n, in;

	if (argc < 4)
	{
		fprintf(stderr, "Usage: %s address port input-filename\n", argv[0]);
		exit(1);
	}

	// set values for server address and port number
	server_address = inet_addr(argv[1]); // server address
	server_port = htons(atoi(argv[2]));	 // port number

	// connect to the swap server
	sd = swap_open(server_address, server_port);
	if (sd < 0)
	{
		fprintf(stderr, "swap_open error\n");
		exit(0);
	}

	// Students to code the following:

	// define the file pointer
	FILE *filePointer;
	// stores each character of input file
	unsigned char fileChar;

	// Allocate memory for the full filename
	unsigned char fullFilename[256];
	unsigned char clientFilePath[256] = "input-client/";
	unsigned char serverFilePath[256] = "output-server/";

	// add folder location

	// Copy argv[3] into fullFilename
	strcpy(fullFilename, argv[3]);

	// Concatenate the file extension
	strcat(fullFilename, ".txt");

	// construct file path for client
	strcat(clientFilePath, fullFilename);

	// construct file path for server
	strcat(serverFilePath, fullFilename);

	printf("\nReading client file: %s\n", clientFilePath);

	// open the file
	filePointer = fopen(clientFilePath, "r");

	// catch errors opening file
	if (NULL == filePointer)
	{
		printf("file can't be opened \n");
	}
	// send input file name to the server
	int serverFilePathLength = strlen(serverFilePath);
	if (swap_write(sd, serverFilePath, serverFilePathLength) != serverFilePathLength)
	{
		printf("Error writing file path to server\n");
		swap_close(sd);
		fclose(filePointer);
		exit(1);
	}

	FILE *file = fopen(clientFilePath, "rb");
	if (!file)
	{
		perror("Failed to open file");
		swap_close(sd);
		exit(1);
	}

	char buffer[MAXLINE];
	size_t bytes_read;

	while ((bytes_read = fread(buffer, 1, MAXLINE, file)) > 0)
	{
		if (swap_write(sd, buffer, bytes_read) < 0)
		{
			fprintf(stderr, "Failed to write data to server.\n");
			fclose(file);
			swap_close(sd);
			exit(1);
		}
	}

	// close the file
	fclose(filePointer);

	// close the connection to the swap server
	swap_close(sd);

	// close connection to input file
	close(in);
}
