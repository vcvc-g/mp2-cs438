/* 
 * File:   sender_main.c
 * Author: 
 *
 * Created on 
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/stat.h>
#include <signal.h>
#include <string.h>
#include <sys/time.h>
#include "sender_helper.h"


struct sockaddr_in si_other;
int s, slen;

void diep(char *s) {
    perror(s);
    exit(1);
}

void* reliablySend(){
    while(1){
        int windowSize = senderInfo->window_size;
        file_data* base = senderInfo->window_packet;
        int sentBytes;
        char sendBuffer[msg_total_size];
        for(int i =0; i < windowSize; i++){
            memset(sendBuffer,'\0', msg_total_size);
            /* need add header in front, currently all data */
            strncpy(sendBuffer, base[i].data, msg_body_size);
            if(i == 0){
                if(base[i].status == -1){
                    sentBytes = snedto(s, sendBuffer, msg_total_size,
                        0, &si_other, sizeof(si_other));
                    base[i].status = 0;
                    gettimeofday(&(senderInfo->timer_start), NULL);
                }

            } else{
                if(base[i].status == -1){
                    sentBytes = snedto(s, sendBuffer, msg_total_size,
                        0, &si_other, sizeof(si_other));
                    base[i].status = 0;

                } else{
                    continue;
                }
            }
        }
    }
}

void* recvACK(){
    int recvBytes;
    char recvBuffer[msg_total_size];
    recvfrom(s, recvBuffer, msg_total_size, 0, &si_other, sizeof(si_other));

    /* read ack from buffer */
    int ACK_seq;

    /* packet sent but not yet ack */
    if(file_data_array[ACK_seq].status == 0){
        // ack packet
        file_data_array[ACK_seq].status = 1;
        //adjust window size
        adjust_window_size();
        // move window base
        senderInfo->window_packet+1;

    }
    /* packet duplicate ack */
    else if(file_data_array[ACK_seq].status == 1){
        senderInfo->dupACK_num += 1;


    }


}

void reliablyTransfer(char* hostname, unsigned short int hostUDPport, char* filename, unsigned long long int bytesToTransfer) {
    //Open the file
    FILE *fp;
    fp = fopen(filename, "rb");
    if (fp == NULL) {
        printf("Could not open file to send.");
        exit(1);
    }

	/* Determine how many bytes to transfer */


    /* Setup UDP connection */
    slen = sizeof (si_other);

    if ((s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1)
        diep("socket");

    memset((char *) &si_other, 0, sizeof (si_other));
    si_other.sin_family = AF_INET;
    si_other.sin_port = htons(hostUDPport);
    if (inet_aton(hostname, &si_other.sin_addr) == 0) {
        fprintf(stderr, "inet_aton() failed\n");
        exit(1);
    }


	/* Send data and receive acknowledgements on s*/

    printf("Closing the socket\n");
    close(s);
    return;

}


/*
 * 
 */
int main(int argc, char** argv) {

    unsigned short int udpPort;
    unsigned long long int numBytes;

    if (argc != 5) {
        fprintf(stderr, "usage: %s receiver_hostname receiver_port filename_to_xfer bytes_to_xfer\n\n", argv[0]);
        exit(1);
    }
    udpPort = (unsigned short int) atoi(argv[2]);
    numBytes = atoll(argv[4]);



    reliablyTransfer(argv[1], udpPort, argv[3], numBytes);


    return (EXIT_SUCCESS);
}


