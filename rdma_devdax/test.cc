#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <infiniband/verbs.h>
#include <sys/mman.h>
#include <unistd.h>

static constexpr size_t kDevdaxFileSize = 2ull * 1024 * 1024 * 1024;

int main() {
  int fd = open("/dev/dax12.0", O_RDWR);
  assert(fd >= 0);

  void *buf =
      mmap(nullptr, kDevdaxFileSize, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
  assert(buf != MAP_FAILED);

  int num_devices = 0;
  struct ibv_device **dev_list = ibv_get_device_list(&num_devices);
  assert(dev_list != nullptr);

  struct ibv_context *ib_ctx = ibv_open_device(dev_list[0]);
  assert(ib_ctx != nullptr);

  struct ibv_pd *pd = ibv_alloc_pd(ib_ctx);
  assert(pd != nullptr);

  int ib_flags = IBV_ACCESS_LOCAL_WRITE | IBV_ACCESS_REMOTE_READ |
                 IBV_ACCESS_REMOTE_WRITE | IBV_ACCESS_REMOTE_ATOMIC;

  struct ibv_mr *mr = ibv_reg_mr(pd, buf, kDevdaxFileSize, ib_flags);
  assert(mr != nullptr);

  // Cleanup devdax buffer
  munmap(buf, kDevdaxFileSize);
  close(fd);
}
