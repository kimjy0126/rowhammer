#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#define PRINT_RES(a, b) printf("Virtual address: %p, Physical address: 0x%lx\n", a, b)

char user_array[1ULL << 30];

size_t virt_to_phys(size_t adrs);
static inline void rowhammer(void *adrs1, void *adrs2);
int main()
{
//    char *user_array;
//    user_array = (char *)malloc(1ULL << 30);
    memset(user_array, 0x00, sizeof(user_array));
    size_t phys_user_array = virt_to_phys((size_t)user_array);

    PRINT_RES(user_array, phys_user_array);
    uint64_t step = 0x1000;
    while (1)
    {
//        printf("Checking for %p...\n", user_array + step);
        volatile size_t phys_step = virt_to_phys((size_t)(user_array + step));
        if ((phys_step - phys_user_array) % 0x800000 == 0)
        {
            PRINT_RES(user_array + step, phys_step);
            break;
        }
        step += 0x1000;
    }
/*    rowhammer(user_array, user_array + step);

    for (uint64_t i = 0; i < 0x100000000ULL; i++)
    {
//        if (user_array[i] & 0xff != 0xff) printf("user_array[%d]: 0xff -> 0x%x\n", i, user_array[i]);
        if (i % 0x100000 == 0) printf("%lx\n", i);
        if (user_array[i]) printf("user_array[%ld]: 0x00 -> 0x%x\n", i, user_array[i]);
    }*/
    free(user_array);
    return 0;
}
size_t virt_to_phys(size_t adrs)
{
    int pagemap = -1;
    pagemap = open("/proc/self/pagemap", O_RDONLY);
    if (pagemap < 0) return -1;

    uint64_t buf;   // 8 bytes
    int got = pread(pagemap, &buf, 8, adrs / 4096 * 8);
    if (got != 8 || buf == 0) return -1;
    uint64_t pf = buf & ((1ULL << 54) - 1);
    if (pf == 0) return -1;
    close(pagemap);
    return pf * 0x1000 + adrs % 0x1000;
}
static inline void rowhammer(void *adrs1, void *adrs2)
{
    asm __volatile__
        (
//         "  1:                      \n"
         "  .rept 5000000           \n"
         "  movq (%0), %%rax        \n"
         "  movq (%1), %%rbx        \n"
         "  clflush (%0)            \n"
         "  clflush (%1)            \n"
         "  mfence                  \n"
         "  .endr                   \n"
//         "  jmp 1                   \n"
         :
         :  "r" (adrs1), "r" (adrs2)
         :  "rax", "rbx"
        );
}
