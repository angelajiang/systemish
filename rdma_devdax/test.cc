#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <infiniband/verbs.h>
#include <sys/mman.h>
#include <unistd.h>

#define MAP_SYNC 0x80000
#define MAP_SHARED_VALIDATE 0x03

static constexpr size_t kDevdaxFileSize = 2ull * 1024 * 1024;
static constexpr const char *kDevDaxFileName = "/dev/dax12.0";

int main() {
  int fd = open(kDevDaxFileName, O_RDWR);
  assert(fd >= 0);

  void *buf = mmap(nullptr, kDevdaxFileSize, PROT_READ | PROT_WRITE,
                   MAP_SHARED | MAP_SYNC, fd, 0);
  if (buf == MAP_FAILED) {
    fprintf(stderr, "mmap failed with error %s\n", strerror(errno));
    exit(-1);
  }

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
  if (mr == nullptr) {
    fprintf(stderr, "ibv_reg_mr failed with error %s\n", strerror(errno));
    exit(-1);
  }
  assert(mr != nullptr);

  // Cleanup devdax buffer
  munmap(buf, kDevdaxFileSize);
  close(fd);
}
