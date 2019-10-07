/* 
 * File:   receiver_main.c
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
#include "receiver_helper.h"

#define FILENAME "recv_output"



struct sockaddr_in si_me, si_other;
int s, slen;

void diep(char *s) {
    perror(s);
    exit(1);
}

FILE *create_file(char *fileName){
	FILE * fPtr = NULL;
	fPtr = fopen(fileName, "wb");
	if (!fPtr )
		printf("create file failed");
	printf("\nrecv_output created\n");
    return fPtr;
}

void *recv_packet(){
    char ACK[msg_total_size]; // ACK msg for sender
    int recvBytes, sentBytes;

    ACK[0] = 'A';

    while(1){
        char recvBuffer[msg_total_size];
	    if ((recvBytes = recvfrom(s, recvBuffer, msg_total_size , 0, &si_me, sizeof(si_me))) == -1) {
            perror("receiver recvfrom failed\n");
            exit(1);
        }

        /* Read header and response */
        // if((SYNACK(seq=y, ACKnum=x+1))&(recvInfo->state == SYN_SENT)){
        //     sendto(ACK(ACKnum=y+1));
        //     recvInfo->state = ESTAB;
        // }

        if(recvInfo->state == ESTAB){
            handle_packet(recvBuffer);
            sentBytes = snedto(s, ACK, msg_total_size, 0, &si_other, sizeof(si_other));
        }

        // else if(){
        //     recvInfo->state = FIN_WAIT2
        // }   
        // else if(){
        //     recvInfo->state = TIME_WAIT
        //     timer 30s
        //     recvInfo->state = CLOSED
        // }           

    }
}


void reliablyReceive(unsigned short int myUDPport, char* destinationFile) {
    
    slen = sizeof (si_other);


    if ((s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1)
        diep("socket");

    memset((char *) &si_me, 0, sizeof (si_me));
    si_me.sin_family = AF_INET;
    si_me.sin_port = htons(myUDPport);
    si_me.sin_addr.s_addr = htonl(INADDR_ANY);
    printf("Now binding\n");
    if (bind(s, (struct sockaddr*) &si_me, sizeof (si_me)) == -1)
        diep("bind");


	/* Create recv_output file */
    create_file(FILENAME);

	/* Now receive data and send acknowledgements */   




    close(s);
	printf("%s received.", destinationFile);
    return;
}

/*
 * 
 */
int main(int argc, char** argv) {

    unsigned short int udpPort;

    if (argc != 3) {
        fprintf(stderr, "usage: %s UDP_port filename_to_write\n\n", argv[0]);
        exit(1);
    }

    udpPort = (unsigned short int) atoi(argv[1]);

    reliablyReceive(udpPort, argv[2]);
}

