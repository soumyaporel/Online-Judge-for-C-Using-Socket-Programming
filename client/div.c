#include <stdio.h>
#include <unistd.h>

void main(int argc, char const *argv[])
{
    //float z = 1 / 0;
    int num1, num2;
    scanf("%d%d", &num1, &num2);
    //int *x;
    //*x = "1";
    int result = num1 / (num2 * 1);
    //int a = 2, b = 0;
    //int c = a / b;
    int arr[10];
    arr[11] = 5;
    printf("%d\n", result);
}
