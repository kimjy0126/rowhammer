#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <time.h>
#include <sys/mman.h>

#define SIZE (1ULL<<30)

double start;
int main(void)
{
    char *memory;
    uint64_t try = 1;

//    memory = (char *)malloc(SIZE * sizeof(char));
    memory = (char *)mmap(NULL, SIZE, PROT_READ | PROT_WRITE, MAP_ANON | MAP_PRIVATE, -1, 0);
    memset(memory, 0xff, SIZE * sizeof(char));

    start = clock();
    while (1)
    {
        printf("try: %lu ", try);
        for (int k = 0; k < 10; k++)
        {
            uint32_t *randaddr[8];
            for (int i = 0; i < 8; i++)
            {
                randaddr[i] = (uint32_t *)(memory + (rand() << 12) % SIZE);
            }
            for (int i = 0; i < 850000; i++)   // very long iteration!
            {
                int dump = 0;
                for (int j = 0; j < 8; j++)
                    dump += *randaddr[j] + 1;
                for (int j = 0; j < 8; j++)
                {
                    asm __volatile__
                        (
                         "  clflush (%0)    \n"
                         :
                         :  "r" (randaddr[j])
                         :
                        );
                }
            }
        }
        // check for bit flips
        for (int i = 0; i < SIZE; i++)
        {
            if (memory[i] + 1)
            {
                printf("bit flip at %p: %x\n", memory + i, memory[i]);
                free(memory);
                return 0;
            }
        }
        try++;
        double end = clock();
        printf("(time elapsed: %lf sec)\n", (end - start) / CLOCKS_PER_SEC);
    }
    free(memory);
    return 0;
}
