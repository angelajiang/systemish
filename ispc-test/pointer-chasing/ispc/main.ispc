#include "common.h"

export void ispc_ptr_chase(uniform int A[], uniform int sum[]) {

    foreach (i = 0 ... BATCH_SIZE) {

		int addr = A[programIndex];

		for(int j = 0; j < DEREF_LENGTH; j++) {
			sum[programIndex] += A[addr];
			addr = (A[addr] + j) & CAP_;
		}
    }
}
