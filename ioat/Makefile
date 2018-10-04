# On the Intel AEP servers, SPDK is installed at system-level, but DPDK isn't.
all:
	g++ -g -o bench bench.cc -L /home/akalia/sandbox/spdk/dpdk/build/lib \
	-lspdk_ioat -lspdk_util -lspdk_env_dpdk -lspdk_log \
	-lpthread -ldpdk -lnuma -ldl -luuid
