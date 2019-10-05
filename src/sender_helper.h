#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#define msg_body_size 1460
#define sender_header_size 3
#define max_seq 720


typedef struct file_data_struct{
    size_t length;
    char* data;
} file_data;



int read_file(char* filename, unsigned long long int bytesToTransfer);





