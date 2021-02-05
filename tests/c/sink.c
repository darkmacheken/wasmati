#include <emscripten.h>
#include <stdio.h>
#include <string.h>
#define MAX 30

extern int source();

extern void sink(int y);

void EMSCRIPTEN_KEEPALIVE foo() {
    int x = source();
    if (x < MAX) {
        int y = 2 * x;
        sink(y);
    }
}
