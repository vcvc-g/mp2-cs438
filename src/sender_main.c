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

void reliablySend(int socket, struct sender_info *sender){
    while(1){
        int windowSize = sender->window_size;
        file_data* base = sender->window_packet;
        int sendBytes;
        for(int i =0; i < windowSize; i++){

            if(i == 0){
                if(base[i].status == -1){
                    sendBytes = snedto(socket, base[i].data, base[i].length,
                        0, &si_other, sizeof(si_other));
                    base[i].status = 0;
                    gettimeofday(&(sender->timer_start), NULL);
                }

            } else{
                if(base[i].status == -1){
                    sendBytes = snedto(socket, base[i].data, base[i].length,
                        0, &si_other, sizeof(si_other));
                    base[i].status = 0;

                } else{
                    continue;
                }
            }
        }
    }
}

void recvACK(int socket, struct sender_info *sender){
    int recvBytes;
    char recvBuffer[msg_total_size];
    recvfrom(socket, recvBuffer, msg_total_size, );


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


