#include <stdio.h>
#include <string.h>
#include <emscripten.h>

extern void allocFunction(int size);

void EMSCRIPTEN_KEEPALIVE callInsecureArgs(char* message) {
    int size = strlen(message);
    allocFunction(size + 1);
}
