#/bin/sh
mkdir -p build
docker run -v $(pwd):/build -t wasmati bash -c "cd /build/build && cmake ../ && cmake --build . --target wasmati"