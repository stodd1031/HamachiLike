#include <stdio.h>
#include <stdlib.h>

int main()
{
    int sum = 0;
    // int arr[] = {1,2,3};
    // int arr[] = {1,3,2};
    // int arr[] = {2,1,3};
    // int arr[] = {2,3,1};
    // int arr[] = {3,1,2};
    // int arr[] = {3,2,1};
    for (int index = 0; index < 3; index++)
    {
        sum += arr[index] * (index+1);
    }
    printf("%i\n", sum);
}