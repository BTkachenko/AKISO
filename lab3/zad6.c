#include <stdio.h>

int main() {
    for (int color = 0; color < 256; color++) {
        printf("\033[38;5;%dmHello, World!\033[0m\n", color);
    }
    return 0;
}
