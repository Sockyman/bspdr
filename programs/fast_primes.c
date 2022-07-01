#include <stdio.h>

#define MAX_COUNT 10000

int main()
{
    int pcount = 0;
    int primes[MAX_COUNT];

    int i = 0;
    while (i < MAX_COUNT)
    {
        primes[i] = 1;
        i = i + 1;
    }

    i = 2;
    while (i < MAX_COUNT)
    {
        if (primes[i])
        {
            printf("%d\n", i);
            pcount = pcount + 1;
            int multiple = i + i;
            while (multiple < MAX_COUNT)
            {
                primes[multiple] = 0;
                multiple = multiple + i;
            }
        }
        i = i + 1;
    }

    printf("total: %d\n", pcount);
}

