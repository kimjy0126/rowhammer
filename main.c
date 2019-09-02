#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#define PRINT_RES(a, b) printf("Virtual address: %p, Physical address: 0x%lx\n", a, b)

char user_array[0x100000000];

size_t virt_to_phys(size_t adrs);
static inline void rowhammer(void *adrs1, void *adrs2);
int main()
{
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
