#include <stdio.h>
#include <stdlib.h>
#include <string.h>

main() {
   FILE *fp;
   int i = 0;

   fp = fopen("./hello.txt", "w+");
   for(i = 0; i < (1500*3); i++){
        fputs("b", fp);
   }
   fclose(fp);
}