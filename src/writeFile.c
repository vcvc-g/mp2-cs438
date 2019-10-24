#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main() {
   FILE *fp;
   int i = 0;
   //(44521*1460)
   fp = fopen("./hello.txt", "w+");
   for(i = 0; i < (44521*1460); i++){
      //char a = i % 10 + 65;
      fputs("A", fp);
      //  if(i < 1460)
      //     fputs("A", fp);
      //  else if(i < 2*1460)
      //     fputs("b", fp);
      //  else if(i < 3*1460)
      //    fputs("c", fp);
      //  else if(i < 4*1460)
      //    fputs("d", fp);
      // else if(i < 5*1460)
      //   fputs("e\n", fp);
      // else if(i < 6*1460)
      //   fputs("f\n", fp);
      // else{
      //    fputs("f\n", fp);
      // }
      //fputs("\n", fp);
   }
   fclose(fp);
}