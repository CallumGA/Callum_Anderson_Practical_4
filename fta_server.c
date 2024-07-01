#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <arpa/inet.h>

#define MAXLINE 128 // maximum bytes to receive and send at once
#define MAX_FTA 512

// External functions
extern int swap_wait(unsigned short port);
extern int swap_read(int sd, unsigned char buf[]);
extern void swap_close(int sd);

int main(int argc, char *argv[])
{
	unsigned short server_port;
	unsigned int server_address;
	unsigned char message[MAXLINE];
	int sd, n, out, bytes_read, bytes_read_contents;
	unsigned char buf[MAX_FTA];
	unsigned char filename[256];
	FILE *file;

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

	// Students to code the following:

	// read in the first message as the new name of the file
	// read message from the client and store them into the above file
	// Read the filename from the client
	bytes_read = swap_read(sd, filename);
	if (bytes_read <= 0)
	{
		fprintf(stderr, "Error reading filename\n");
		exit(EXIT_FAILURE);
	}
	filename[bytes_read] = '\0'; // Null-terminate the filename

	// Open the file for writing
	file = fopen((const char *)filename, "wb");
	if (!file)
	{
		fprintf(stderr, "Error opening file: %s\n", filename);
		exit(EXIT_FAILURE);
	}

	// Read file data from the client until end-of-file
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
