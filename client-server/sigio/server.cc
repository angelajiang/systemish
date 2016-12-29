#include <arpa/inet.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/syscall.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <string>
#include <thread>
#define gettid() syscall(SYS_gettid)

using namespace std;

#define my_port 3490 /* The port users will be sending to */
#define MAXBUFLEN 100

#define DELIVER_TO_MAIN 1 /* Deliver SIGIO to only main thread */

void io_handler(int); /* This is the I/O handler */

/* Data shared between main() and io_handler() */
int sock_fd;
volatile int counter = 0;

__thread std::string *thread_name;

void secondary() {
  printf("secondary thread. gettid: %ld\n", gettid());
  thread_name = new std::string("secondary");

  int prev_counter = 0;
  for (;;) {
    while (prev_counter == counter) {
      // Busy wait
    }

    // printf("secondary thread: Counter changed to %d\n", counter);
    prev_counter = counter;
  }
}

int main() {
  printf("main thread. gettid: %ld\n", gettid());
  thread_name = new std::string("main");

  /*
   * Create a UDP socket.
   * AF_INET = IPv4, SOCK_DGRAM = datagrams, IPPROTO_UDP = datagrams over UDP.
   * Returns a file descriptor.
   */
  sock_fd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
  if (sock_fd < 0) {
    perror("Error opening datagram socket");
    exit(1);
  }

  /*
   * Bind the socket to accept packets destined to any IP interface of this
   * machine (INADDR_ANY), and to port @my_port.
   */
  struct sockaddr_in server;
  memset(&server, 0, sizeof(struct sockaddr_in));
  server.sin_family = AF_INET;
  server.sin_addr.s_addr = INADDR_ANY;
  server.sin_port = htons(my_port);

  if (bind(sock_fd, (struct sockaddr *)&server, sizeof(struct sockaddr_in)) <
      0) {
    perror("Error binding datagram socket");
    exit(1);
  }

  /* Set up a SIGIO signal handler */
  signal(SIGIO, io_handler);

#if DELIVER_TO_MAIN == 1
  /* Ensure that only the main thread receives SIGIO */
  struct f_owner_ex owner_thread;
  owner_thread.type = F_OWNER_TID;
  owner_thread.pid = gettid();

  if (fcntl(sock_fd, F_SETOWN_EX, &owner_thread) < 0) {
    perror("Error: fcntl F_SETOWN_EX");
    exit(1);
  }
#else
  if (fcntl(sock_fd, F_SETOWN, getpid()) < 0) {
    perror("Error: fcntl F_SETOWN");
    exit(1);
  }
#endif

  /* Set file flags. Allow receipt of asynchronous I/O signals */
  if (fcntl(sock_fd, F_SETFL, O_ASYNC) < 0) {
    perror("Error: fcntl F_SETFL, FASYNC");
    exit(1);
  }

  /* Launch the secondary thread */
  std::thread secondary_thread(secondary);

  /* The main code */
  int prev_counter = 0;
  for (;;) {
    while (prev_counter == counter) {
      // Busy wait
    }

    // printf("main: Counter changed to %d\n", counter);
    prev_counter = counter;
  }

  secondary_thread.join();
}

void io_handler(int signal) {
  int numbytes;                  /* Number of bytes recieved from client */
  uint32_t addr_len;             /* Address size of the sender		*/
  struct sockaddr_in their_addr; /* Connector's address information	*/
  char buf[MAXBUFLEN];           /* The buffer to receive the data into */

  printf("\n%s: io_handler() calling recv_from().\n", thread_name->c_str());

  if ((numbytes = recvfrom(sock_fd, buf, MAXBUFLEN, 0,
                           (struct sockaddr *)&their_addr, &addr_len)) == -1) {
    printf("%s: recvfrom() error\n", thread_name->c_str());
    exit(1);
  }

  buf[numbytes] = '\0';
  counter++;
  printf("%s: io_handler got from %s: %s. Setting counter = %d\n",
         thread_name->c_str(), inet_ntoa(their_addr.sin_addr), buf, counter);
  return;
}
