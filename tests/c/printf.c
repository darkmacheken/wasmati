#include <stdio.h>
#include <emscripten.h>

void EMSCRIPTEN_KEEPALIVE logMessage(char* message) {
    printf("%s", message);
}

void EMSCRIPTEN_KEEPALIVE logMessageVuln(char* message) {
    printf(message);
}