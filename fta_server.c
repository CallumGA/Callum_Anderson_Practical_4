/*
 * COMP 3271 - Computer Networks
 * Project Part 4 - Basic file transfer application using SWAP protocol
 * Modifications - Implement usage of the swap_read, swap_wait, and swap_close functions
 *
 * Author: Callum Anderson
 * Date: July 2, 2024
 *
 * Description:
 * Uses the wait, read, and close SWAP functions to read incoming packets and save the contents and file to the server.
 *
 * To Compile:   gcc fta_server.c swap_server.c sdp.c checksum.c -o fta_server
 * To Run:       ./fta_server 8899
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
#define MAX_FTA 512

// external swap functions
extern int swap_wait(unsigned short port);
extern int swap_read(int sd, unsigned char buf[]);
extern void swap_close(int sd);

int main(int argc, char *argv[])
{
	// declare variables for the following code
	unsigned short server_port;
	unsigned int server_address;
	unsigned char message[MAXLINE];
	int sd, n, out, bytes_read, bytes_read_contents;
	unsigned char buf[MAX_FTA];
	unsigned char filename[256];
	FILE *file;

	// verify correct command line params passed
	if (argc < 2)
	{
		fprintf(stderr, "Usage: %s port\n", argv[0]);
		exit(1);
	}

	// set values for server address and port number
	server_port = htons(atoi(argv[1]));

	// connect to the port allocated to the swap server
	sd = swap_wait(server_port);
	if (sd < 0)
	{
		fprintf(stderr, "swap_open error\n");
		exit(0);
	}

	// read the incoming file name from the client
	bytes_read = swap_read(sd, filename);
	// error handling for read
	if (bytes_read <= 0)
	{
		fprintf(stderr, "Error reading filename\n");
		exit(EXIT_FAILURE);
	}
	// null-terminate the filename
	filename[bytes_read] = '\0';

	// use the sent filename to create or open the file for writing
	file = fopen((const char *)filename, "wb");
	// error handling for opening the file
	if (!file)
	{
		fprintf(stderr, "Error opening file: %s\n", filename);
		exit(EXIT_FAILURE);
	}

	// read file data from the packets being sent from the client
	while ((bytes_read_contents = swap_read(sd, buf)) > 0)
	{
		// write incoming packet data to the file on the server
		fwrite(buf, 1, bytes_read_contents, file);
	}

	// close the connection to the swap servfer
	swap_close(sd);

	// close connection to output file
	close(out);
}
