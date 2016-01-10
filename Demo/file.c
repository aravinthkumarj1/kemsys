#include <stdio.h>
#define MAXBUFLEN 1000000
void main()
{
char source[MAXBUFLEN + 1];
FILE *fp = fopen("ipc.c", "r");
if (fp != NULL) {
    size_t newLen = fread(source, sizeof(char), MAXBUFLEN, fp);
    if (newLen == 0) {
        fputs("Error reading file", stderr);
    } else {
        source[newLen++] = '\0'; /* Just to be safe. */
    }

    fclose(fp);
}
}
