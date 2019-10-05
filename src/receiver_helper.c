#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>

#include "receiver_helper.h"

void write_file(char *packet, FILE* fptr){

    if(fptr){
        fwrite(&packet[sender_header_size], msg_body_size, 1, fptr);
        puts("Packet write into file\n");
    }


}