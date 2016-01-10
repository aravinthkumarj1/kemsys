#include<stdio.h>
// function prototype, also called function declaration
 
int main()
{
    int m = 22, n = 44;
    // calling swap function by value
    printf(" values before swap  m = %d \nand n = %d\n", m, n);
    swap();                         
}
 
void swap()
{ 
    int a=1,b=1;
    printf(" \nvalues after swap m = %d\n and n = %d\n", a, b);
}
