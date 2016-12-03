#!/bin/bash

# Start lsync from Haswell server (47) to all client machines
# For machines that are down, or to which 47 does not have passwordless access,
# this will fail silently.

servers="46 48 49 50 51 52 65 66 91 92"
sync_dirs="rdma_bench hots accelio"

local_server=47
for i in $servers; do
  if [ "$local_server" -ne "$i" ]; then
    for dir in $sync_dirs; do
      lsyncd -delay .5 -rsync $dir anuj@10.113.1.$i:/home/anuj/$dir
    done
	fi
done

echo "Waiting 5 seconds before printing lsync status"
sleep 5
for i in $servers; do
	ps -afx | grep lsyncd | grep "10\.113\.1\.$i"
done
