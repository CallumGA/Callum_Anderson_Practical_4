/*
 * COMP 3271 - Computer Networks
 * Project Part 3 - Programming at the Data Link Layer
 * Modifications - Implementation of the swap_write function
 *
 * Author: Callum Anderson
 * Date: June 29, 2024 *
 *
 * Description:
 * This program implements a client for the SWAP protocol at the Data Link Layer.
 * It establishes a connection to the server, sends data frames, and handles acknowledgments.
 *
 * To Compile:   gcc test_swap_client.c swap_client.c sdp.c checksum.c -o test_swap_client
 * To Run:       ./test_swap_client 127.0.0.1 9001
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>

#define MAXLINE 512  // Maximum characters to receive and send at once
#define MAXFRAME 512 // Maximum frame size

extern int swap_connect(unsigned int addr, unsigned short port);
extern int swap_disconnect(int sd);
extern int sdp_send(int sd, char *buf, int length);
extern int sdp_receive(int sd, char *buf);
extern int sdp_receive_with_timer(int sd, char *buf, unsigned int expiration);
unsigned short int checksum(unsigned char buf[], int length);

int session_id = 0; // Session ID for the current connection
int S = 0;          // Frame number to send, toggles between 0 and 1

int swap_open(unsigned int addr, unsigned short port)
{
    int sockfd;                  // Socket descriptor
    struct sockaddr_in servaddr; // Server address
    char buf[MAXLINE];
    int len, n;

    // If a session is already open, return an error
    if (session_id != 0)
        return -1;

    // Connect to a server on the specified address and port
    session_id = swap_connect(addr, port); // Defined in sdp.o

    // Return the session ID
    return session_id;
}

int swap_write(int sd, char *buf, int length)
{
    // Check if the session ID is valid and matches the current session
    if (session_id == 0 || sd != session_id)
    {
        return -1;
    }

    char frame[MAXFRAME];
    unsigned short chksum;
    // Sequence byte + data length will be frame length (checksum not added yet)
    int frame_length = length + 1;
    char ack[MAXFRAME];

    do
    {
        // sequence number
        frame[0] = (char)S;
        // copy buffer data into the frame from right after the sequence
        memcpy(&frame[1], buf, length);

        // pad odd number frame length with exta byte for checksum
        if (frame_length % 2 != 0)
        {
            frame[frame_length] = 0;
            frame_length++;
        }

        // Construct the frame: [sequence 1 byte][data][checksum 2 byte]

        // Calculate checksum which will be added to end of the frame
        chksum = checksum((unsigned char *)frame, frame_length);
        // Checksum high byte
        frame[frame_length] = (char)(chksum >> 8);
        // Checksum low byte
        frame[frame_length + 1] = (char)(chksum & 0xFF);

        printf("Sending frame with sequence number %d...\n", S);
        // Send the frame with checksum, adding 2 for the checksum addition
        sdp_send(sd, frame, frame_length + 2);

        // Wait 5000 ms for acknowledgment
        int ack_length = sdp_receive_with_timer(sd, ack, 10000);
        // if we see any errors timeout and try retransmitting
        if (ack_length == -1)
        {
            printf("Timeout, retransmitting...\n");
            // Timeout, retransmit
            continue;
        }

        // If everything is all good, we can process the received ACK
        if (ack[0] == (char)S)
        {
            printf("Received acknowledgment for sequence %d\n", S);
            // Toggle the sequence number (0 or 1)
            S = ((char)S + 1) % 2;
            break;
        }
        else
        {
            printf("Incorrect acknowledgment received, retransmitting...\n");
        }

    } while (1);

    return length;
}

void swap_close(int sd)
{
    // Check if the session ID is valid and matches the current session
    if (session_id == 0 || sd != session_id)
        return;

    session_id = 0; // Reset the session ID

    swap_disconnect(sd); // Disconnect the session (defined in sdp.o)
}