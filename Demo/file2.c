#include <stdio.h>
#include <string.h>

int main()
{
   FILE *fp;
   char buffer[10000];

   /* Open file for both reading and writing */
   fp = fopen("ipc.c", "r");

   fread(buffer, sizeof(buffer), 1, fp);
   printf("%s\n", buffer);
   fclose(fp);
   
   return(0);
}
