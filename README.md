# perlin_noise
emcc worker.cpp -o ../../temp/workerWasm.js     -s MODULARIZE=1     -s EXPORT_NAME='createModule'     -s USE_PTHREADS=1     -s PTHREAD_POOL_SIZE=0     -s  WASM_MEM_MAX=512MB     --bind

perf stat -d ../../temp/./worker 1000 1000 300 1 123123

add cpuid

sudo cpupower frequency-set --governor performance

taskset -c 0 ./scripts/wasm/worker2

g++ scripts/wasm/worker2.cpp -msse4.1 -o scripts/wasm/worker2

clang++ scripts/wasm/worker2.cpp -o scripts/wasm/worker2

sudo taskset -c 0 perf record -e cycles,instructions,cache-references,cache-misses,branches,branch-misses ./scripts/wasm/worker2

sudo perf report

lldb ./scripts/wasm/worker2

gdb ./scripts/wasm/worker2
