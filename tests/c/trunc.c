#include <emscripten.h>
#include <limits.h>

int EMSCRIPTEN_KEEPALIVE foo(long long int i) {
    return i + 1;
}
