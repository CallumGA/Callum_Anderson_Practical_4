/*
 * COMP 3271 - Computer Networks
 * Project Part 4 - Basic file transfer application using SWAP protocol
 * Modifications - Implement usage of the swap_write function for transfering a file from client to server
 *
 * Author: Callum Anderson
 * Date: July 2, 2024
 *
 * Description:
 * Uses the open, write, and close SWAP functions to transfer a file via SWAP protocol to the server
 *
 * To Compile:   gcc fta_client.c swap_client.c sdp.c checksum.c -o fta_client
 * To Run:       ./fta_client 127.0.0.1 8899 input-filename
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <arpa/inet.h>

// maximum bytes to receive and send at once
#define MAXLINE 128

// external swap functions
extern int swap_open(unsigned int addr, unsigned short port);
extern int swap_write(int sd, unsigned char buf[], int length);
extern void swap_close(int sd);

int main(int argc, char *argv[])
{
	// declare variables for following code
	unsigned short server_port;
	unsigned int server_address;
	unsigned char buf[MAXLINE];
	int sd, n, in;
	char buffer[MAXLINE];
	size_t bytes_read;

	// verify we are getting minimum of 4 command line params passed in
	if (argc < 4)
	{
		fprintf(stderr, "Usage: %s address port input-filename\n", argv[0]);
		exit(1);
	}

	// set values for server address and port number
	server_address = inet_addr(argv[1]);
	server_port = htons(atoi(argv[2]));

	// connect to the swap server
	sd = swap_open(server_address, server_port);
	if (sd < 0)
	{
		fprintf(stderr, "swap_open error\n");
		exit(0);
	}

	// define file pointer
	FILE *filePointer;
	// declare full file name
	unsigned char fullFilename[256];
	// declare client file path
	unsigned char clientFilePath[256] = "input-client/";
	// declare server file path
	unsigned char serverFilePath[256] = "output-server/";

	// copy passed in file name to variable
	strcpy(fullFilename, argv[3]);

	// add file extension to the file name
	strcat(fullFilename, ".txt");

	// construct file path for client file location (for reading)
	strcat(clientFilePath, fullFilename);

	// construct file path for server file location (for sending to server)
	strcat(serverFilePath, fullFilename);

	printf("\nReading client file: %s\n", clientFilePath);

	// open the file on the client to get file name
	filePointer = fopen(clientFilePath, "rb");

	// catch errors opening file
	if (NULL == filePointer)
	{
		printf("file can't be opened \n");
	}
	// get length of the server file path
	int serverFilePathLength = strlen(serverFilePath);
	// send the file name with extension to the server
	if (swap_write(sd, serverFilePath, serverFilePathLength) != serverFilePathLength)
	{
		printf("Error writing file path to server\n");
		swap_close(sd);
		fclose(filePointer);
		exit(1);
	}

	// while loop for sending read file contents to the server
	// terminates at EOF
	while ((bytes_read = fread(buffer, 1, MAXLINE, filePointer)) > 0)
	{
		// error handling
		if (swap_write(sd, buffer, bytes_read) < 0)
		{
			fprintf(stderr, "Failed to write data to server.\n");
			fclose(filePointer);
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
