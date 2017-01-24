// Measure the time to signal a thread using condition variables
#include <pthread.h>
#include <stdio.h>
#include <time.h>
#include <unistd.h>

#include <thread>

#define ITERS 1000000

pthread_cond_t cond = PTHREAD_COND_INITIALIZER;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

int num_jobs = 0;

void thread_func() {
  int tot_jobs = 0;

  /*
   * pthread_cond_wait() requires the thread to hold @mutex before the call.
   * @mutex is released on entering the wait, and is re-acquired before
   * returning from the wait.
   * 
   * Missed signals: All jobs enqueued by main before this thread's first call
   * to pthread_cond_wait() are processed before this thread's first call
   * to pthread_cond_wait().
   */
  pthread_mutex_lock(&mutex);

  while (true) {
    /* We always hold @mutex before modifying @num_jobs */
    if (num_jobs > 0) {
      /* Process the new jobs */
      tot_jobs += num_jobs;
      num_jobs = 0;

      /* Quit when we have processed all jobs */
      if (tot_jobs == ITERS) {
        return;
      }
    }

    /* This unlocks @mutex */
    pthread_cond_wait(&cond, &mutex);

    /* Here, we hold @mutex again */
  }
}

int main() {
  std::thread worker_thread(thread_func);
  struct timespec start, end;
  double nanoseconds = 0;

  for (int i = 0; i < ITERS; i++) {
    /* Enqueue one job */
    pthread_mutex_lock(&mutex);
    num_jobs++;
    pthread_mutex_unlock(&mutex);

    /* Signal the worker after releasing the mutex */
    clock_gettime(CLOCK_REALTIME, &start);
    pthread_cond_signal(&cond);
    clock_gettime(CLOCK_REALTIME, &end);

    nanoseconds += (end.tv_sec - start.tv_sec) * 1000000000 +
                   (end.tv_nsec - start.tv_nsec);
  }

  printf("Time per notify_one() = %f ns\n", nanoseconds / ITERS);
  worker_thread.join();
}
