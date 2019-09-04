#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <inttypes.h>
#include <sys/mman.h>

#define SIZE    (1ULL << 30)    // memory size

#define AI  55                  // activation interval (ns)
#define RI  6400000             // refresh interval (ns)
// #define N   (2 * (RI) / (AI))   // number of toggles
#define N 850000

char user_array[SIZE];

static inline void rowhammer(uint32_t *adrsset[], int n);

int main()
{
    memset(user_array, 0xff, sizeof(user_array));

    uint64_t try = 0;

    while (1)
    {
        printf("try: %lu\n", try++);
        uint32_t *adrsset[8];
        for (int i = 0; i < 10; i++)
        {
            for (int j = 0; j < 8; j++)
                adrsset[j] = (uint32_t *)(user_array + (rand() << 12) % SIZE);
            rowhammer(adrsset, 8);
        }
        uint64_t *end = (uint64_t *)(user_array + SIZE);
        uint64_t *ptr;
        for (ptr = (uint64_t *)user_array; ptr < end; ptr++)
        {
            if (*ptr + 1)
            {
                printf("flipped at %p: 0xffffffffffffffff -> 0x%" PRIx64 "\n", ptr, *ptr);
                return 0;
            }
        }
    }
    return 0;
}
static inline void rowhammer(uint32_t *adrsset[], int n)
{
/*    for (int i = 0; i < N; i++)
    {
        int sum = 0;
        for (int j = 0; j < n; j++)
            sum += *adrsset[j] + 1;
        for (int j = 0; j < n; j++)
            asm __volatile__ (" clflush (%0)    \n" : : "r" (adrsset[j]) : "memory" );

        if (sum != 0)
        {
            printf("thats nono\n");
        }
    }*/
    for (int i = 0; i < N; i++)
    {
        for (int j = 0; j < n; j++)
        {
            asm __volatile__
                (
                 "  movq (%0), %%rax    \n"
                 :
                 :  "r" (adrsset[j])
                 :  "rax"
                );
        }
        for (int j = 0; j < 8; j++)
        {
            asm __volatile__
                (
                 "  clflush (%0)        \n"
                 :
                 :  "r" (adrsset[j])
                 :
                );
        }
        asm __volatile__ (" mfence  \n" : : : );
    }
/*    for (int i = 0; i < 8; i++)
    {
        printf("%lu\n", adrsset[i]);
    }*/
}
