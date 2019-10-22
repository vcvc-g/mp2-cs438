#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main() {
   FILE *fp;
   int i = 0;

   fp = fopen("./hello.txt", "w+");
   for(i = 0; i < (1465*20); i++){
        fputs("b\n", fp);
   }
   fputs("==================================\n", fp);
   fclose(fp);
}