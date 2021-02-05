#include <emscripten.h>
#include <stdio.h>
#include <stdlib.h>


extern int source();
extern void sink(int y);
extern int predicate();

void EMSCRIPTEN_KEEPALIVE foo() {
    char* pointer = malloc(source());
    printf("%s", pointer);
    if (predicate()) {
        free(pointer);
    }
}
