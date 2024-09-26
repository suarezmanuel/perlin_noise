# perlin_noise
emcc worker.cpp -o ../../temp/workerWasm.js     -s MODULARIZE=1     -s EXPORT_NAME='createModule'     -s USE_PTHREADS=1     -s PTHREAD_POOL_SIZE=0     -s  WASM_MEM_MAX=512MB     --bind

perf stat -d ../../temp/./worker 1000 1000 300 1 123123

add cpuid

sudo cpupower frequency-set --governor performance

taskset -c 0 ./scripts/wasm/worker2