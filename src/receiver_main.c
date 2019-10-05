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




struct sockaddr_in si_me, si_other;
int s, slen;

void diep(char *s) {
    perror(s);
    exit(1);
}


void recv_packet(){
    int recvBytes;
    while(1){
        char recvBuffer[msg_total_size];
	    if ((recvBytes = recvfrom(s, recvBuffer, msg_total_size , 0, &si_me, sizeof(si_me))) == -1) {
            perror("receiver recvfrom failed\n");
            exit(1);
        }
        // if(recieve_packetNumer  == window_base){
            // start_timer;
            // expect_seq = base + 1;
            // pending_ack = window_base;
            // start_timer;
        //                         }
        // else if(recieve_packetNumer  == pending_ack + 1){
            // for(to find the largest connected recieve number)
            //     send_ack[largest connected recieve number];
            //     base = recieve_packetNumer + 1;
            //     last_ack = recieve_packetNumer;
            //     pending_ack = -1;
        // }
        // else if(recieve_packetNumer !=  expect_seq)
            // if(pending_ack = -1)
            //         send_ack[last_ack];
            // else  
            //         send_ack[pending_ack];
        

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
	FILE * fPtr = NULL;
	fPtr = fopen("recv_output", "wb");
	if (!fPtr )
		printf("create file failed");
	printf("\nrecv_output created\n");

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

