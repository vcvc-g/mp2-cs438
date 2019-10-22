#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main() {
   FILE *fp;
   int i = 0;
   //(44521*1460)
   fp = fopen("./hello.txt", "w+");
   for(i = 0; i < (10*1460); i++){
        fputs("d", fp);
   }
   fclose(fp);
}