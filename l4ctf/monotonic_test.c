#include "../fiasco/ktrace_events.h"
#include <stdint.h>
#include <stdio.h>

#define BUF_SIZE 1025

int main()
{
    //FILE *file = fopen("/tmp/l4trace.out", "r");
    FILE *file = fopen("/tmp/l4trace_sorted.out", "r");

    l4_tracebuffer_entry_t buf[BUF_SIZE];
    l4_tracebuffer_entry_t last;
    int count;

    uint64_t gc = 0;
    uint64_t gm = 0;

    while((count = fread(buf, sizeof(l4_tracebuffer_entry_t), BUF_SIZE, file)) > 0) {
        for (int i = 0; i < count; i++) {
            gc++;
            if (last._kclock > buf[i]._kclock || (last._kclock == buf[i]._kclock && last._number > buf[i]._number)) {
                gm++;
                printf("[%d]: kclock=%u (cpu=%hhu,tsc=%llu,type=%hhu,number=%lx) klock=%u (cpu=%hhu,tsc=%llu,type=%hhu,number=%lx) diff=%lx (%lx %lx)\n",
                       i,
                       last._kclock, last._cpu, last._tsc, last._type, last._number,
                       buf[i]._kclock, buf[i]._cpu, buf[i]._tsc, buf[i]._type, buf[i]._number,
                       last._number - buf[i]._number,
                       buf[i-2]._number, buf[i+1]._number);
            }
            last = buf[i];
        }
    }
    printf("gc=%llu", gc);
    printf("gm=%llu", gm);

    fclose(file);

    return 0;
}

