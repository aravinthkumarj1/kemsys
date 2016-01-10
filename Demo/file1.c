#include <stdio.h>
void main()
{
FILE *p;
char ch;
p=fopen("ipc.c","r");
ch=getc(p);
putchar(ch);
fclose(p);
}
