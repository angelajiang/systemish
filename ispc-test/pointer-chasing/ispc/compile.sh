ISPC_HOME="/home/akalia/ispc-v1.8.0-linux"

# Remove array A's shm-id
sudo ipcrm -M 3185

rm main

# Create C++ file
ispc main.ispc --emit-c++ --cpu=corei7-avx --target=generic-16 -o main-ispc.cpp --c++-include-file=generic-16.h

gcc -O3 -I $ISPC_HOME/examples/intrinsics/ main-ispc.cpp main.c -o main -lrt -lm


