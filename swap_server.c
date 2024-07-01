/*
 * COMP 3271 - Computer Networks
 * Project Part 3 - Programming at the Data Link Layer
 * Modifications - Implementation of the swap_read function
 *
 * Author: Callum Anderson
 * Date: June 29, 2024 *
 *
 * Description:
 * This program implements a server for the SWAP protocol at the Data Link Layer.
 * It listens for incoming connections, receives frames, checks for errors using checksums,
 * and sends acknowledgments for correctly received frames.
 *
 * To Compile:   gcc test_swap_server.c swap_server.c sdp.c checksum.c -o test_swap_server
 * To Run:       ./test_swap_server 9001
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <Kernel/string.h>

#define MAXLINE 512  // Maximum characters to receive and send at once
#define MAXFRAME 512 // Maximum frame size

extern int swap_accept(unsigned short port);
extern int swap_disconnect(int sd);
extern int sdp_send(int sd, char *buf, int length);
extern int sdp_receive(int sd, char *buf);
extern int sdp_receive_with_timer(int sd, char *buf, unsigned int expiration);
unsigned short int checksum(unsigned char buf[], int length);

// Session ID for the current connection
int session_id = 0;
// Frame number to receive, toggles between 0 and 1
int R = 0;

int swap_wait(unsigned short port)
{
    printf("Waiting...");

    // If a session is already open, return an error
    if (session_id != 0)
        return -1;

    // Accept a connection on the specified port
    session_id = swap_accept(port);

    // Return the session ID
    return session_id;
}

int swap_read(int sd, char *buf)
{
    // Check if the session ID is valid and matches the current session
    if (session_id == 0 || sd != session_id)
    {
        return -1;
    }

    char frame[MAXFRAME];
    // Clear the frame buffer
    memset(frame, 0, sizeof(frame));

    // Receive data into the frame buffer
    int n = sdp_receive(sd, frame);
    // Return error if reception failed
    if (n < 0)
    {
        return n;
    }

    // Return error for invalid frame length
    if (n < 3)
    {
        printf("Invalid frame length\n");
        return -1;
    }

    // Extract the received checksum from the frame (we must get the last 2 bytes and ignore the sequence and data itself)
    unsigned short recv_chksum = ((unsigned char)frame[n - 2] << 8) | (unsigned char)frame[n - 1];
    // Calculate the checksum of the received frame (just the sequence and the data, leaving out the received checksum)
    unsigned short calc_chksum = checksum((unsigned char *)frame, n - 2);

    printf("\nReceived checksum: %u, Calculated checksum: %u\n", recv_chksum, calc_chksum);
    printf("Received sequence: %d, Expected sequence: %d\n", frame[0], R);

    // Check if the calculated checksum matches the received checksum and sequence number is as expected
    if (calc_chksum == recv_chksum && frame[0] == (char)R)
    {
        // Copy data to buffer, excluding sequence and checksum
        memcpy(buf, &frame[1], n - 3);
        // Toggle the expected sequence number
        R = (R + 1) % 2;

        // Send acknowledgment with the received sequence number
        char ack[2] = {frame[0], 0};
        sdp_send(sd, ack, 2);

        printf("Acknowledgment sent for sequence: %d\n", frame[0]);

        // Return the length of data itself (minus 3 for sequnece + checksum)
        return n - 3;
    }

    // Return error for invalid frame or sequence mismatch
    printf("Invalid frame or sequence number mismatch\n");
    return -1;
}

void swap_close(int sd)
{
    // Check if the session ID is valid and matches the current session
    if (session_id == 0 || sd != session_id)
        return;

    // Reset the session ID
    session_id = 0;
    // Disconnect the session
    swap_disconnect(sd);
}