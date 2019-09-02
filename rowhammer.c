#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <inttypes.h>
#include <sys/mman.h>

static int size = 1ULL << 30;
char *user_array;

void find_proper_addresses(void *adrs1, void *adrs2);
static inline void rowhammer(void *adrs1, void *adrs2);

int main()
{
    void *adrs1, *adrs2;
    user_array = (char *)mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_ANON | MAP_PRIVATE, -1, 0);
    memset(user_array, 0xff, sizeof(user_array));
    find_proper_addresses(adrs1, adrs2);
    memset(user_array, 0xff, sizeof(user_array));

    rowhammer(adrs1, adrs2);

    uint64_t try = 0;

    while (1)
    {
        printf("try: %lu\n", try++);
        rowhammer(adrs1, adrs2);
        uint64_t *end = (uint64_t *)(user_array + size);
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
}
static inline void rowhammer(void *adrs1, void *adrs2)
{
    asm __volatile__
        (
         "  .rept 1000000           \n"
         "  movq (%0), %%rax        \n"
         "  movq (%1), %%rbx        \n"
         "  clflush (%0)            \n"
         "  clflush (%1)            \n"
         "  mfence                  \n"
         "  .endr                   \n"
         :
         :  "r" (adrs1), "r" (adrs2)
         :  "rax", "rbx"
        );
}
