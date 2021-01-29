#include <emscripten.h>
#include <stdio.h>
#include <stdlib.h>


extern int source();
extern void sink(int y);
extern int predicate();

void EMSCRIPTEN_KEEPALIVE foo() {
    char* pointer = malloc(source());
    if (predicate()) {
        free(pointer);
    }
    printf("%s", pointer);
}
