#include <stdlib.h>

unsigned int random_next = 1;
int rand() {
    random_next *= 0x41c64e6d;
    random_next += 0x3039;
    return (random_next >> 16) & 0x7FFF;
}
void srand(unsigned int seed) {
    random_next = seed;
}
