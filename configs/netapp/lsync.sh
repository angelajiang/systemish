#!/bin/bash

# Start lsync from Haswell server (46) to all client machines
# For machines that are down, or to which the Haswell does not have passwordless
# access, this will fail silently.

for i in `seq 36 45`; do
	lsyncd -delay .5 -rsync rdma_bench anuj@10.113.1.$i:rdma_bench
done
