#include "common.h"

int main(void) {
    if (getuid() != 0) {
        fprintf(stderr, "Need ROOT\n");
        exit(EXIT_FAILURE);
    }
    return 0;
}